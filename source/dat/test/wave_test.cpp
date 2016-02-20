#include "../wave.hpp"
#include "../../common/events.hpp"
#include <catch.hpp>
#include <tinythread.h>

using scigma::dat::WaveUpdateEvent;
using scigma::dat::Wave;
using scigma::dat::Mesh;

using scigma::common::EventSink;
class WaveUpdateSink: public EventSink<WaveUpdateEvent>::Type
{
public:
  size_t count;
  WaveUpdateSink():count(0){}
  virtual bool process(WaveUpdateEvent e){++count;return true;}
};


SCENARIO("Wave/Mesh: reading and writing in a single thread","[Wave][Mesh][single-thread]")
{
  GIVEN("A Wave or Mesh object initialized with a single value and capacity 1")
    {
      Wave w(1);
      Mesh m(1);

      w.push_back(3.14);
      m.push_back(3);

      THEN("size() returns 1")
	{
	  REQUIRE(w.size()==1);
	  REQUIRE(m.size()==1);
	}
      THEN("the value is retrieved with data()[0]")
	{
	  REQUIRE(w.data()[0]==3.14);
	  REQUIRE(m.data()[0]==3);
	}
      WHEN("a range of values is pushed")
	{
	  double D[]={1,2,3,4,5};
	  uint32_t I[]={1,2,3,4,5};
	  w.push_back(D,5);
	  m.push_back(I,5);
	  THEN("the size increases by the number of values")
	    {
	      REQUIRE(w.size()==6);
	      REQUIRE(m.size()==6);
	    }
	  THEN("the values are retrieved with data()[...]")
	    {
	      const double* ddata=w.data();
	      const uint32_t* idata=m.data();
	      for(size_t i(0);i<5;++i)
		{
		  REQUIRE(ddata[i+1]==D[i]);
		  REQUIRE(idata[i+1]==I[i]);
		}
	    }
	  THEN("popping a single value reduces the size by one")
	    {
	      w.pop_back();
	      m.pop_back();
	      REQUIRE(w.size()==5);
	      REQUIRE(m.size()==5);
	    }
	  THEN("popping multiple values reduces the size accordingly")
	    {
	      w.pop_back(3);
	      m.pop_back(3);
	      REQUIRE(w.size()==3);
	      REQUIRE(m.size()==3);
	    }
	}
      THEN("whenever values are pushed or popped, WaveUpdateEvent is emitted")
	{
	  WaveUpdateSink ws,ms;
	  w.connect(&ws);
	  m.connect(&ms);

	  double D[]={1,2,3,4,5};
	  uint32_t I[]={1,2,3,4,5};

	  w.pop_back();m.pop_back();
	  w.push_back(D,5);m.push_back(I,5);
	  w.push_back(3.14);m.push_back(3);
	  w.pop_back(2);m.pop_back(2);

	  REQUIRE(4==ws.count);
	  REQUIRE(4==ms.count);
	}
    }
}

void read(void* data)
{
  Wave* w = static_cast<Wave*>(data);
  while(true)
    {
      w->lock();
      const double* d(w->data());
      size_t s(w->size());
      if(s==400)
	break;
      for(size_t i(0);i<s;++i)
	if(d[i]<0){w=NULL;}
      w->unlock();
    }
}

void write(void* data)
{
  Wave* w = static_cast<Wave*>(data);
  double d[]={1,2,3};
  for(size_t i(0);i<100;++i)
    {
      w->push_back(d,3);
      w->pop_back();
    }
}


SCENARIO("Wave/Mesh: writing in one thread and reading from another","[Wave][Mesh][multi-thread]")
{
  GIVEN("An empty Wave or Mesh object, two threads that write to it and one thread that reads from it")
    {
      Wave w;

      tthread::thread thread1(write,static_cast<void*>(&w));
      tthread::thread thread2(write,static_cast<void*>(&w));
      tthread::thread thread3(read,static_cast<void*>(&w));

      thread1.join();
      thread2.join();
      thread3.join();

      THEN("the threads interact without deadlocks and the final state of the data is as expected")
	{
	  REQUIRE(w.size()==400);
	  const double* d(w.data());
	  size_t count1(0),count2(0),count3(0);
	  for(size_t i(0);i<400;++i)
	    {
	      if(1==d[i])++count1;
	      if(2==d[i])++count2;
	      if(3==d[i])++count3;
	    }
	    REQUIRE(200==count1);
	    REQUIRE(200==count2+count3);
	}
    }
}
