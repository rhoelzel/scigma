#include <limits>
#include <cmath>
#include "mesh.hpp"
#include <iostream>

namespace scigma
{
  namespace dat
  {
    
    Mesh::Mesh(size_t nDim, const std::vector<double>& initial):
      PythonObject<Mesh>(this),
      nDim_(nDim),
      lastStripTriangleDataBegin_(0),currentStripTriangleDataBegin_(nDim*NVALS_PER_DIM),
      lastStripTriangleIndicesBegin_(0),currentStripTriangleIndicesBegin_(1)
    {
      triangleData_.lock();
      triangleIndices_.lock();

      for(size_t i(0);i<nDim;++i)
	{
	  triangleData_.push_back(initial[i]);
	  triangleData_.push_back(0);
	  triangleData_.push_back(0);
	  triangleData_.push_back(0);
	}

      triangleIndices_.push_back(0);
      
      triangleIndices_.unlock();
      triangleData_.unlock();

      
      std::vector<double> initialRing;
      for(size_t i(nDim_);i<initial.size();++i)
	initialRing.push_back(initial[i]);

      add_layer(initialRing);
      lastStripTriangleDataBegin_=currentStripTriangleDataBegin_;
      lastStripTriangleIndicesBegin_=currentStripTriangleIndicesBegin_;
      currentStripTriangleDataBegin_=triangleData_.size();
      currentStripTriangleIndicesBegin_=triangleIndices_.size();


      isoIndices_.lock();
      isoIndices_.push_back(0);
      GLint start((GLint(isoIndices_.size())));
      GLint end(start+(GLint(initial.size()/nDim_)));
      isoIndices_.push_back(-start);
      for(GLint i(start+1);i<end-1;++i)
	isoIndices_.push_back(i);
      isoIndices_.push_back(-(end-1));
      isoIndices_.unlock();
    }
    
    void Mesh::add_strip(const std::vector<double>& positions)
    {
      triangleData_.lock();
      triangleIndices_.lock();
  
      /* add the new layer of points */
      add_layer(positions);

      /* compute correct normal information, for the next to last layer of points, 
	 replacing the old normal information of the boundary layer */
      compute_normal_information();

      /* invalidate the last part of the triangleData_ Wave, so that the new
	 normal information will be reflected in attached buffers */
      triangleData_.invalidate_from_index(lastStripTriangleDataBegin_);
      
      /* update internal variables */
      lastStripTriangleDataBegin_=currentStripTriangleDataBegin_;
      lastStripTriangleIndicesBegin_=currentStripTriangleIndicesBegin_;
      currentStripTriangleDataBegin_=triangleData_.size();
      currentStripTriangleIndicesBegin_=triangleIndices_.size();

      triangleIndices_.unlock();
      triangleData_.unlock();      

      /* add indices for the ISOLINES drawing style; 	 
	 make first and last index of layer negative,
	 so that the shader knows where to skip fragments
      */
      isoIndices_.lock();
      GLint start((GLint(isoIndices_.size())));
      GLint end(start+(GLint(positions.size()/nDim_)));
      isoIndices_.push_back(-start);
      for(GLint i(start+1);i<end-1;++i)
	isoIndices_.push_back(i);
      isoIndices_.push_back(-(end-1));
      isoIndices_.unlock();

    }
    
    const IWave& Mesh::triangle_indices() const {return triangleIndices_;}
    const IWave& Mesh::iso_indices() const {return isoIndices_;}
	
    const Wave& Mesh::triangle_data() const {return triangleData_;}

    double Mesh::distance_squared(GLint index1, GLint index2, const double* tData) const
    {
      double result(0);
      for(size_t i(0);i<nDim_;++i)
	{
	  double x1(tData[(size_t(index1)*nDim_+i)*NVALS_PER_DIM]);
	  double x2(tData[(size_t(index2)*nDim_+i)*NVALS_PER_DIM]);
	  result+=(x2-x1)*(x2-x1);
	}
      return result;
    }

    void Mesh::add_layer(const std::vector<double>& positions)
    {
      /* create room for positions and normal information in triangleData_ */
      std::vector<double> zeroes(positions.size()*NVALS_PER_DIM,0);
      triangleData_.push_back(&zeroes[0],zeroes.size());

      /* enter positions of the points */
      double* tData(triangleData_.data());
      for(size_t i(0),size(positions.size());i<size;++i)
	tData[currentStripTriangleDataBegin_+i*NVALS_PER_DIM]=positions[i];

      /* find the point in the new layer that is closest to the last
	 point of the old layer; this will become the first point
	 of the new layer */
      GLint nCurrent((GLint(positions.size()/nDim_)));
      GLint* iData(triangleIndices_.data());
      GLint lastLastIndex(iData[lastStripTriangleIndicesBegin_]);
      GLint firstCurrentIndex((GLint(currentStripTriangleDataBegin_/nDim_/NVALS_PER_DIM)));

      double d2Min(std::numeric_limits<double>::max());
      for(GLint i(firstCurrentIndex), max(firstCurrentIndex+nCurrent);i<max;++i)
	{
	  double d2(distance_squared(lastLastIndex,i,tData));
	  if(d2<d2Min)
	    {
	      d2Min=d2;
	      firstCurrentIndex=i;
	      if(d2<=0.0)
		break;
	    }
	}

      /* add the indices for the new triangle strip; the idea is to start at 
	 firstCurrentIndex and alternate between the last layer and the current layer
	 until we end up at firstCurrentIndex again. The diagram below illustrates 
	 the behavior if everything goes smoothly: (-1) is the last point of the
	 old layer, (0) is the point at firstCurrentIndex, which is the first point we
	 add.

	     (0)---(2)--           (n-1)--(0)
	     / \   / \       ...      \   /
	    /   \ /   \                \ /
	   (-1)--(1)---(3)--         --(-1)
	 
	 if the layers do not fit perfectly (for example, they might differ in the
	 number of points), we double back occasionally, preferring short diagonals 
	 over long diagonals, i.e.:

	      (0)---(2)-(4)                         (0)---(2)-(4)
	      / \   /  .´ `.       instead of       / \   / `-. `.
	     /   \ / .´     `.                     /   \ /     `-.`.
	   (-1)--(1,3)-------(5)                 (-1)--(1)---------(3)

      */
      triangleIndices_.push_back(firstCurrentIndex);

      /* these macros are abbreviations for the modulo logic that
	 is used for cycling through layers */
      GLint lastIndexOffset((GLint(lastStripTriangleDataBegin_/nDim_/NVALS_PER_DIM)));
      GLint currentIndexOffset((GLint(currentStripTriangleDataBegin_/nDim_/NVALS_PER_DIM)));
      GLint nLast(currentIndexOffset-lastIndexOffset);
      
#define INC_LAST(x) ((x+1-lastIndexOffset)%nLast+lastIndexOffset)
#define INC_CURR(x) ((x+1-currentIndexOffset)%nCurrent+currentIndexOffset)       
#define DEC_LAST(x) ((x-1-lastIndexOffset+nLast)%nLast+lastIndexOffset)
#define DEC_CURR(x) ((x-1-currentIndexOffset+nCurrent)%nCurrent+currentIndexOffset)       
      
      GLint currentIndex(INC_CURR(firstCurrentIndex));
      GLint lastIndex(INC_LAST(iData[lastStripTriangleIndicesBegin_]));

      bool addWithoutCheck=false;
      
      while(currentIndex!=firstCurrentIndex||lastIndex!=lastLastIndex)
	{
	  /* first, we are looking for a point on the last layer */

	  /* compute the two possible diagonals for the next triangle 
	     (if needed, see addWithoutCheck further down) */

	  double diag1(1),diag2(0);
	  if((!addWithoutCheck)&&(currentIndex!=firstCurrentIndex))
	    {
	      diag1=distance_squared(currentIndex,DEC_LAST(lastIndex),tData);
	      diag2=distance_squared(DEC_CURR(currentIndex),lastIndex,tData);
	      std::cout<<"current1="<<currentIndex<<", last1="<<DEC_LAST(lastIndex)<<std::endl;
	      std::cout<<"current2="<<DEC_CURR(currentIndex)<<", last2="<<lastIndex<<std::endl;
	      std::cout<<"diag1="<<diag1<<", diag2="<<diag2<<std::endl;
	    }
	  addWithoutCheck=false;
	  
	  if(diag1>=diag2)
	    {
	      /* take the next point on the last layer */
	      triangleIndices_.push_back(lastIndex);
	      lastIndex=(lastIndex==lastLastIndex)?lastIndex:INC_LAST(lastIndex);
	    }
	  else
	    {
	      /* double back to the same point on the old layer */
	      triangleIndices_.push_back(DEC_LAST(lastIndex));
	      /* add the next point on the new layer and go to
		 next iteration */
	      triangleIndices_.push_back(currentIndex);
	      currentIndex=(currentIndex==firstCurrentIndex)?currentIndex:INC_CURR(currentIndex);
	      continue;
	    }
	  
	  /* now, we are looking for a point on the new layer */

	  /* compute the two possible diagonals for the next triangle */
	  diag1=0;diag2=1;
	  if(lastIndex!=lastLastIndex)
	    {
	      diag1=distance_squared(currentIndex,DEC_LAST(lastIndex),tData);
	      diag2=distance_squared(DEC_CURR(currentIndex),lastIndex,tData);
	    } 
	  if(diag1<=diag2)
	    {
	      /* take the next point on the new layer */
	      triangleIndices_.push_back(currentIndex);
	      currentIndex=(currentIndex==firstCurrentIndex)?currentIndex:INC_CURR(currentIndex);
	    }
	  else
	    {
	      /* double back to the same point on the new layer */
	      triangleIndices_.push_back(DEC_CURR(currentIndex));
	      /* mark the next point on the old layer for instant
		 addition */
	      addWithoutCheck=true;
	    }
	}

      /* complete the layers (here, we also allow for the long
	 diagonal instead of the short diagonal, because it will
	 have no effect cascading further down the strip) */
      triangleIndices_.push_back(lastLastIndex);
      triangleIndices_.push_back(firstCurrentIndex);
      
      /* add the index of the start (and end) point another time,
	 to avoid triangles spanning three layers; this is equivalent
	 to an additional doubling back in both this triangle strip
	 and in the new one that will be added;
      */
      triangleIndices_.push_back(firstCurrentIndex);
    }

    void Mesh::collect_neighbours_before(GLint beginIndex, GLint endIndex, std::map<GLint, std::vector<GLint> >& neighbours)
    {
      /* skip the two last indices of the layer, as they were only added for drawing reasons
	 and their information on neighbours is guaranteed to be redundant */
      GLint nIndices(endIndex-beginIndex-2);

      GLint* iData(triangleIndices_.data());

      GLint* offset(&iData[beginIndex]);
      GLint* index(offset);

      /* start on a fresh index (so we know there are no new neighbours
	 coming up once we went full circle around the layer;
	 this will produce errors if there is only one point in the 
	 layer, unfortunately */
      
      while(*index==*offset)
	  index+=2;
      GLint* prevIndex(index-2);
      GLint* startingIndex(index); 

      bool passed(false);
      
      while(true)
	{
	  /* if one cycle is completed, stop */
	  if(index==startingIndex)
	    {
	      if(passed)
		break;
	      passed=true;
	    }

	  if(*index!=*prevIndex)
	    {
	      /* whenever we come across a new point, add the two previous points in 
		 triangle strip to the list of neighbours (i.e. the point immediately
		 preceding it on its own layer, and its first neighbour on the layer 
		 below */
	      neighbours[index[0]].push_back(prevIndex[0]);
	      neighbours[index[0]].push_back(prevIndex[1]);
	    } 
	  /* add next point in triangle strip to the list of neighbours, if it 
	     is not already in there */
	  if((neighbours[index[0]].size()==0)||
	     (neighbours[index[0]].back()!=index[1]))
	    neighbours[index[0]].push_back(index[1]);

	  prevIndex=index;
	  index=offset+(index-offset+2)%nIndices;
	}

      /* at this point, neighbours contains all the neighbours of points
	 in the layer that are in the layer before, plus the point
	 that is immediately preceding it on its own layer, in correct order */
    }

    void Mesh::collect_neighbours_after(GLint beginIndex, GLint endIndex, std::map<GLint, std::vector<GLint> >& neighbours)
    {
      /* skip first and last index of the layer */
      GLint nIndices(endIndex-beginIndex-2);
      GLint* iData(triangleIndices_.data());

      GLint* offset(&iData[beginIndex]);
      GLint* index(offset+nIndices-2);

      /* start on a fresh index (so we know there are no new neighbours
	 coming up once we went full circle around the last layer;
	 this will produce errors if there is only one point in the last
	 layer, unfortunately;
	 to get the correct order, we traverse the layer backwards */
      
      while(index[1]==offset[1])
	  index-=2;
      GLint* prevIndex(offset+(index-offset+2)%nIndices);
      GLint* startingIndex(index); 

      bool passed(false);
      
      while(true)
	{
	  /* if one cycle is completed, stop */
	  if(index==startingIndex)
	    {
	      if(passed)
		break;
	      passed=true;
	    }

	  if(index[1]!=prevIndex[1])
	    {
	      /* whenever we come across a new point, add the two previous points in 
		 triangle strip to the list of neighbours (i.e. the point immediately
		 preceding it on its own layer, and its first neighbour on the layer 
		 below */
	      neighbours[index[1]].push_back(prevIndex[1]);
	      neighbours[index[1]].push_back(prevIndex[0]);
	    } 
	  /* add next point in triangle strip to the list of neighbours, if it 
	     is not already in there */
	  if((neighbours[index[1]].size()==0)||
	     (neighbours[index[1]].back()!=index[0]))
	    neighbours[index[1]].push_back(index[0]);

	  prevIndex=index;
	  index=offset+(index-offset+nIndices-2)%nIndices;
	}

      /* at this point, neighbours contains all the neighbours of points
	 in the layer that are in the layer after, plus the point
	 that is immediately following it on its own layer, in correct order */

    }
    
    void Mesh::compute_normal_information()
    {
      //      GLint nCurrent((GLint(triangleIndices_.size()-currentStripTriangleIndicesBegin_-2)));
      
      /* update normal information for last layer */

      std::map<GLint,std::vector<GLint> > neighbours;
      collect_neighbours_before(GLint(lastStripTriangleIndicesBegin_),GLint(currentStripTriangleIndicesBegin_),neighbours);
      collect_neighbours_after(GLint(currentStripTriangleIndicesBegin_),GLint(triangleIndices_.size()),neighbours);

      typedef std::map<GLint,std::vector<GLint> >::iterator It;
      It begin(neighbours.begin()), end(neighbours.end());
      
      for(It i(begin);i!=end;++i)
	{
	  std::cout<<"Neighbours for "<<i->first<<":"<<std::endl;
	  for(size_t j(0);j<i->second.size();++j)
	    std::cout<<i->second[j]<<", ";
	  std::cout<<std::endl;
	}

      /* average a triangle from the neighbours of each index, which will be used
	 by the shader to compute a normal */
      
      double* tData(triangleData_.data());
      for(It i(begin);i!=end;++i)
	compute_triangle_for_normal(i->first,i->second);
    }

    void Mesh::compute_triangle_for_normal(GLint index, std::vector<double>& neighbourIndices)
    {
      double n((double(neighbourIndices.size())));
      double nGroup(n/3.0);

      for(double i(1);i<n+1;++i)
	{
	  double rest(fmod(i,nGroup));
	  double weight(rest<1?1-rest:1);
	}
    }
    
  } /* end namespace dat */
} /* end namespace scigma */
