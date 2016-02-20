#ifndef SCIGMA_DAT_WAVE_HPP
#define SCIGMA_DAT_WAVE_HPP

#include <tinythread.h>
#include "../common/pythonobject.hpp"
#include "../common/events.hpp"

using scigma::common::PythonObject;
using scigma::common::EventSource;

namespace scigma
{
  namespace dat
  {
    struct WaveUpdateEvent
    { //!Argument list for ScaleEvent 
      typedef LOKI_TYPELIST_0 Arguments;
    };
    
    template <class T> class AbstractWave:
      public PythonObject<AbstractWave<T>>,
      public EventSource<WaveUpdateEvent>::Type
      {
    public:
    	AbstractWave(size_t capacity=0x1000);
	~AbstractWave();
	
	void lock();
	void unlock();

	// size() is not threadsafe -> use only between lock() and unlock() !!
	size_t size() const;
	// data() is not threadsafe -> use only between lock() and unlock() !!
	const T* data() const;
	
	void push_back(T value);
	void push_back(T* values, size_t nValues);
      
	void pop_back();
	void pop_back(size_t nValues);

    private:
      AbstractWave(const AbstractWave<T>&);
      AbstractWave& operator=(const AbstractWave<T>&);

      size_t size_;
      size_t capacity_;
      T* data_;
      T* oldData_;

      mutable tthread::mutex mutex_;
    };

    template <class T> AbstractWave<T>::AbstractWave(size_t capacity):
      PythonObject<AbstractWave<T>>(this),size_(0),capacity_(capacity<2?2:capacity),data_(new T[capacity_]),oldData_(data_)
    {}

    template <class T> AbstractWave<T>::~AbstractWave()
    {
      delete [] data_;
    }

    template <class T> size_t AbstractWave<T>::size() const {return size_;}

    template <class T> void AbstractWave<T>::lock(){mutex_.lock();}
    template <class T> void AbstractWave<T>::unlock(){mutex_.unlock();}
    
    template <class T> const T* AbstractWave<T>::data() const {return data_;}

    template <class T> void AbstractWave<T>::push_back(T value)
    {
      push_back(&value,1);
    }

    template <class T> void AbstractWave<T>::push_back(T* values, size_t nValues)
    {
      lock();

      if(capacity_<size_+nValues)
	{
	  while(capacity_<size_+nValues)
	    {
	      capacity_*=1.5;
	    }
	  T* d = new T[capacity_];
	  for(size_t i(0);i<size_;++i)
	    d[i]=data_[i];
	  data_=d;
	  delete [] oldData_;
	  oldData_=data_;
	}
      for(size_t i(0);i<nValues;++i)
	data_[size_+i]=values[i]; 
      size_+=nValues;

      unlock();
      emit();
    }
              
    template <class T> void AbstractWave<T>::pop_back()
    {
      pop_back(1);
    }
    
    template <class T> void AbstractWave<T>::pop_back(size_t nValues)
    {
      lock();
      size_=size_>nValues?size_-nValues:0;
      unlock();
      emit();
    }


    typedef AbstractWave<double> Wave;
    typedef AbstractWave<uint32_t> Mesh;
    
  } /* end namespace dat */
} /* end namespace scigma */

#endif /* SCIGMA_DAT_WAVE_HPP */
