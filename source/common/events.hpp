#ifndef SCIGMA_COMMON_EVENTS_HPP
#define SCIGMA_COMMON_EVENTS_HPP

#include <vector>
#include <algorithm>
#include <Typelist.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"

namespace scigma
{
  namespace common
  {
  
    template <class T> struct Event
    {
      typedef LOKI_TYPELIST_0 ArgumentList;
      //or, for one argument:
      //typedef LOKI_TYPELIST_1(ArgumentType1) ArgumentList;
      //or, for two arguments:
      //typedef LOKI_TYPELIST_0(ArgumentType1, ArgumentType2) ArgumentList;
      //...
    };

    // template for EventSink objects: inherit from EventSink<Event>::Type
    template <class Event, unsigned int nArguments> class EventSinkWithNArguments;
    template <class Event> struct EventSink
    {
      typedef EventSinkWithNArguments<Event,Loki::TL::Length<typename Event::Arguments>::value> Type;
    };

    // template for EventSource objects: inherit from EventSource<Event>::Type
    template <class Event, unsigned int nArguments> class EventSourceWithNArguments;
    template <class Event> struct EventSource
    {
      typedef EventSourceWithNArguments<Event,Loki::TL::Length<typename Event::Arguments>::value> Type;
    };


    // free connect and disconnect functions
    template <class Event> void connect_before(typename EventSource<Event>::Type* source,
					       typename EventSink<Event>::Type* sink)
    {
      source->sinks_.insert(source->sinks_.begin(),sink);
      sink->sources_.push_back(source);
    }

    template <class Event> void connect(typename EventSource<Event>::Type* source,
					typename EventSink<Event>::Type* sink)
    {
      source->sinks_.push_back(sink);
      sink->sources_.push_back(source);
    }

    template <class Event> void disconnect(typename EventSource<Event>::Type* source,
					   typename EventSink<Event>::Type* sink)
    {
      auto iSink=std::find(source->sinks_.begin(),source->sinks_.end(),sink); 
      if(iSink!=source->sinks_.end())
	source->sinks_.erase(iSink);
      auto iSource=std::find(sink->sources_.begin(),sink->sources_.end(),source); 
      if(iSource!=sink->sources_.end())
	sink->sources_.erase(iSource);
    }

    
    template <class Event> class EventSinkWithNArguments<Event,0>
    {
      friend void connect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void connect_before<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void disconnect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      
    private:
      std::vector<typename EventSource<Event>::Type*> sources_;
    public:
      virtual bool process(Event event)=0;
      ~EventSinkWithNArguments<Event,0>()
      {
	std::vector<typename EventSource<Event>::Type*> sources(sources_);
	typename std::vector<typename EventSource<Event>::Type*>::iterator iSource = sources.begin();
	while(iSource!=sources.end())
	  {
	    disconnect<Event>(*iSource,this);
	    ++iSource;
	  }
      }
    };
  
    template <class Event> class EventSinkWithNArguments<Event,1>
    {
      friend void connect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void connect_before<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void disconnect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
    
    private:
      std::vector<typename EventSource<Event>::Type*> sources_;
    public:
      virtual bool process(Event event,
			   typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1)=0;
      ~EventSinkWithNArguments<Event,1>()
      {
	std::vector<typename EventSource<Event>::Type*> sources(sources_);
	typename std::vector<typename EventSource<Event>::Type*>::iterator iSource = sources.begin();
	while(iSource!=sources.end())
	  {
	    disconnect<Event>(*iSource,this);
	    ++iSource;
	  }
      }
    };
    
    template <class Event> class EventSinkWithNArguments<Event,2>
    {
      friend void connect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void connect_before<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void disconnect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);

    private:
      std::vector<typename EventSource<Event>::Type*> sources_;
    public:
      virtual bool process(Event event,
			   typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1,
			   typename Loki::TL::TypeAt<typename Event::Arguments,1>::Result arg2)=0;
      ~EventSinkWithNArguments<Event,2>()
      {
	std::vector<typename EventSource<Event>::Type*> sources(sources_);
	typename std::vector<typename EventSource<Event>::Type*>::iterator iSource = sources.begin();
	while(iSource!=sources.end())
	  {
	    disconnect<Event>(*iSource,this);
	    ++iSource;
	  }
      }
    };
    
    template <class Event> class EventSinkWithNArguments<Event,3>
    {
      friend void connect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void connect_before<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void disconnect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);

    private:
      std::vector<typename EventSource<Event>::Type*> sources_;
    public:
      virtual bool process(Event event,
			   typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1,
			   typename Loki::TL::TypeAt<typename Event::Arguments,1>::Result arg2,
			   typename Loki::TL::TypeAt<typename Event::Arguments,2>::Result arg3)=0;
      ~EventSinkWithNArguments<Event,3>()
      {
	std::vector<typename EventSource<Event>::Type*> sources(sources_);
	typename std::vector<typename EventSource<Event>::Type*>::iterator iSource = sources.begin();
	while(iSource!=sources.end())
	  {
	    disconnect<Event>(*iSource,this);
	    ++iSource;
	  }
      }
    };
    
    template <class Event> class EventSinkWithNArguments<Event,4>
    {
      friend void connect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void connect_before<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void disconnect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);

    private:
      std::vector<typename EventSource<Event>::Type*> sources_;
    public:
      virtual bool process(Event event,
			   typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1,
			   typename Loki::TL::TypeAt<typename Event::Arguments,1>::Result arg2,
			   typename Loki::TL::TypeAt<typename Event::Arguments,2>::Result arg3,
			   typename Loki::TL::TypeAt<typename Event::Arguments,3>::Result arg4)=0;
      ~EventSinkWithNArguments<Event,4>()
      {
	std::vector<typename EventSource<Event>::Type*> sources(sources_);
	typename std::vector<typename EventSource<Event>::Type*>::iterator iSource = sources.begin();
	while(iSource!=sources.end())
	  {
	    disconnect<Event>(*iSource,this);
	    ++iSource;
	  }
      }
    };
    
    template <class Event> class EventSinkWithNArguments<Event,5>
    {
      friend void connect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void connect_before<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void disconnect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);

    private:
      std::vector<typename EventSource<Event>::Type*> sources_;
    public:
      virtual bool process(Event event,
			   typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1,
			   typename Loki::TL::TypeAt<typename Event::Arguments,1>::Result arg2,
			   typename Loki::TL::TypeAt<typename Event::Arguments,2>::Result arg3,
			   typename Loki::TL::TypeAt<typename Event::Arguments,3>::Result arg4,
			   typename Loki::TL::TypeAt<typename Event::Arguments,4>::Result arg5)=0;
      ~EventSinkWithNArguments<Event,5>()
      {
	std::vector<typename EventSource<Event>::Type*> sources(sources_);
	typename std::vector<typename EventSource<Event>::Type*>::iterator iSource = sources.begin();
	while(iSource!=sources.end())
	  {
	    disconnect<Event>(*iSource,this);
	    ++iSource;
	  }
      }
    };
    
        
  template <class Event, unsigned int nArguments> class EventSourceWithNArguments; 
    
    template <class Event> class EventSourceWithNArguments<Event,0>
    {
      friend void connect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void connect_before<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void disconnect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);

    protected:
      std::vector<typename EventSink<Event>::Type*> sinks_;
      void emit()
      {
	for(typename std::vector<typename EventSink<Event>::Type *>::iterator i=sinks_.begin(),end=sinks_.end();i!=end;++i)
	  if((*i)->process(Event()))
	    return;
      }
    public:
      ~EventSourceWithNArguments<Event,0>()
      {
	std::vector<typename EventSink<Event>::Type*> sinks(sinks_);
	typename std::vector<typename EventSink<Event>::Type*>::iterator iSink = sinks.begin();
	while(iSink!=sinks.end())
	  {
	    disconnect<Event>(this,*iSink);
	    ++iSink;
	  }
      }
    };
  
    template <class Event> class EventSourceWithNArguments<Event,1>
    {
      friend void connect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void connect_before<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void disconnect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);

    protected:
      std::vector<typename EventSink<Event>::Type*> sinks_;
      void emit(typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1)
      {
	for(typename std::vector<typename EventSink<Event>::Type *>::iterator i=sinks_.begin(),end=sinks_.end();i!=end;++i)
	  if((*i)->process(Event(), arg1))
	    return;
      }
    public:
      ~EventSourceWithNArguments<Event,1>()
      {
	std::vector<typename EventSink<Event>::Type*> sinks(sinks_);
	typename std::vector<typename EventSink<Event>::Type*>::iterator iSink = sinks.begin();
	while(iSink!=sinks.end())
	  {
	    disconnect<Event>(this,*iSink);
	    ++iSink;
	  }
      }

    };
    
    template <class Event> class EventSourceWithNArguments<Event,2>
    {
      friend void connect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void connect_before<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void disconnect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);

    protected:
      std::vector<typename EventSink<Event>::Type*> sinks_;
      void emit(typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1,
		typename Loki::TL::TypeAt<typename Event::Arguments,1>::Result arg2)
      {
	for(typename std::vector<typename EventSink<Event>::Type *>::iterator i=sinks_.begin(),end=sinks_.end();i!=end;++i)
	  if((*i)->process(Event(), arg1, arg2))
	    return;
      }
    public:
      ~EventSourceWithNArguments<Event,2>()
      {
	std::vector<typename EventSink<Event>::Type*> sinks(sinks_);
	typename std::vector<typename EventSink<Event>::Type*>::iterator iSink = sinks.begin();
	while(iSink!=sinks.end())
	  {
	    disconnect<Event>(this,*iSink);
	    ++iSink;
	  }
      }

    };
    
    template <class Event> class EventSourceWithNArguments<Event,3>
    {
      friend void connect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void connect_before<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void disconnect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);

    protected:
      std::vector<typename EventSink<Event>::Type*> sinks_;
      void emit(typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1,
		typename Loki::TL::TypeAt<typename Event::Arguments,1>::Result arg2,
		typename Loki::TL::TypeAt<typename Event::Arguments,2>::Result arg3)
      {
	for(typename std::vector<typename EventSink<Event>::Type *>::iterator i=sinks_.begin(),end=sinks_.end();i!=end;++i)
	  if((*i)->process(Event(), arg1, arg2, arg3))
	    return;
      }
    public:
      ~EventSourceWithNArguments<Event,3>()
      {
	std::vector<typename EventSink<Event>::Type*> sinks(sinks_);
	typename std::vector<typename EventSink<Event>::Type*>::iterator iSink = sinks.begin();
	while(iSink!=sinks.end())
	  {
	    disconnect<Event>(this,*iSink);
	    ++iSink;
	  }
      }

    };
    
  template <class Event> class EventSourceWithNArguments<Event,4>
  {
    friend void connect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
    friend void connect_before<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
    friend void disconnect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
    
  protected:
    std::vector<typename EventSink<Event>::Type*> sinks_;
    void emit(typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1,
	      typename Loki::TL::TypeAt<typename Event::Arguments,1>::Result arg2,
	      typename Loki::TL::TypeAt<typename Event::Arguments,2>::Result arg3,
	      typename Loki::TL::TypeAt<typename Event::Arguments,3>::Result arg4)
    {
      for(typename std::vector<typename EventSink<Event>::Type *>::iterator i=sinks_.begin(),end=sinks_.end();i!=end;++i)
	if((*i)->process(Event(), arg1, arg2, arg3, arg4))
	  return;
    }
  public:
      ~EventSourceWithNArguments<Event,4>()
      {
	std::vector<typename EventSink<Event>::Type*> sinks(sinks_);
	typename std::vector<typename EventSink<Event>::Type*>::iterator iSink = sinks.begin();
	while(iSink!=sinks.end())
	  {
	    disconnect<Event>(this,*iSink);
	    ++iSink;
	  }
      }

  };
    
    template <class Event> class EventSourceWithNArguments<Event,5>
    {
      friend void connect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void connect_before<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);
      friend void disconnect<Event>(typename EventSource<Event>::Type*,typename EventSink<Event>::Type*);

    protected:
      std::vector<typename EventSink<Event>::Type*> sinks_;
      void emit(typename Loki::TL::TypeAt<typename Event::Arguments,0>::Result arg1,
		typename Loki::TL::TypeAt<typename Event::Arguments,1>::Result arg2,
		typename Loki::TL::TypeAt<typename Event::Arguments,2>::Result arg3,
		typename Loki::TL::TypeAt<typename Event::Arguments,3>::Result arg4,
		typename Loki::TL::TypeAt<typename Event::Arguments,4>::Result arg5)
      {
	for(typename std::vector<typename EventSink<Event>::Type *>::iterator i=sinks_.begin(),end=sinks_.end();i!=end;++i)
	  if((*i)->process(Event(), arg1, arg2, arg3, arg4, arg5))
	    return;
      }
    public:
      ~EventSourceWithNArguments<Event,5>()
      {
	std::vector<typename EventSink<Event>::Type*> sinks(sinks_);
	typename std::vector<typename EventSink<Event>::Type*>::iterator iSink = sinks.begin();
	while(iSink!=sinks.end())
	  {
	    disconnect<Event>(this,*iSink);
	    ++iSink;
	  }
      }

    };
    
    
  } /* end namespace gui */ 
} /* end namespace scigma */ 

#pragma clang diagnostic pop
  
#endif /* SCIGMA_COMMON_EVENTS_HPP */
