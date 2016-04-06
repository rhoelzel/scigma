#ifndef SCIGMA_DAT_MESH_HPP
#define SCIGMA_DAT_MESH_HPP

#include <vector>
#include <map>
#include "../common/pythonobject.hpp"
#include "../gui/definitions.hpp"
#include "wave.hpp"

using scigma::common::PythonObject;
typedef scigma::dat::AbstractWave<double> Wave;
typedef scigma::dat::AbstractWave<GLint> IWave;

namespace scigma
{
  namespace dat
  {
    class Mesh:
      public PythonObject<Mesh>
      {
      public:
	Mesh(size_t nDim, const std::vector<double>& initial);

	void add_strip(const std::vector<double>& positions);

	const IWave& triangle_indices() const;
	const IWave& iso_indices() const;
	
	const Wave& triangle_data() const;
	
      private:
	const size_t NVALS_PER_DIM = 4;

	Mesh(const Mesh&);
	Mesh& operator=(const Mesh&);

	IWave triangleIndices_, isoIndices_;
	Wave triangleData_;

	size_t nDim_;
	size_t lastStripTriangleDataBegin_,currentStripTriangleDataBegin_;
	size_t lastStripTriangleIndicesBegin_,currentStripTriangleIndicesBegin_;
	
	double distance_squared(GLint index1, GLint index2, const double* tData) const;

	void add_layer(const std::vector<double>& positions);

	void collect_neighbours_before(GLint beginIndex, GLint endIndex, std::map<GLint, std::vector<GLint> >& neighbours);
	void collect_neighbours_after(GLint beginIndex, GLint endIndex, std::map<GLint, std::vector<GLint> >& neighbours);

	void compute_triangle_for_normal(GLint index, std::vector<double>& neighbourIndices);
	
	void compute_normal_information();
	
    };

  } /* end namespace dat */
} /* end namespace scigma */

#endif /* SCIGMA_DAT_MESH_HPP */
