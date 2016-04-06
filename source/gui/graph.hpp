#ifndef SCIGMA_GUI_GRAPH_HPP
#define SCIGMA_GUI_GRAPH_HPP

#include <string>
#include <vector>
#include <map>
#include "../common/events.hpp"
#include "definitions.hpp"
#include "marker.hpp"

using scigma::common::EventSource;

namespace scigma
{
  namespace gui
  {
    struct GraphClickEvent{  
      typedef LOKI_TYPELIST_2(const char*,int) Arguments;};
    struct PointClickEvent{  
      typedef LOKI_TYPELIST_2(const char*,int) Arguments;};

    class GLContext;
    class GLWindow;

    class Graph:
      public EventSource<GraphClickEvent>::Type,
      public EventSource<PointClickEvent>::Type
    {
   
    public:

      typedef std::vector<std::string> VecS;
      typedef std::map<GLContext*, std::vector<GLfloat> > LightMap;
      
      enum Style
	{
	  POINTS,
	  LINES,
	  ISOLINES,
	  WIREFRAME,
	  SOLID
	};

      virtual void set_marker_style(Marker::Type marker);
      Marker::Type marker_style() const;
      
      virtual void set_marker_size(GLfloat size);
      GLfloat marker_size() const;
      
      virtual void set_point_style(Marker::Type point);
      Marker::Type point_style() const;

      virtual void set_point_size(GLfloat size);
      GLfloat point_size() const;

      virtual void set_color(const GLfloat* color);
      const GLfloat* color() const;
      
      virtual void set_delay(GLfloat delay);
      GLfloat delay() const;

      virtual void set_style(Style style);
      GLfloat style() const;

      virtual void replay()=0;
      virtual void finalize()=0;

      virtual void set_attributes_for_view(const std::vector<size_t>& varyingBaseIndex,
					   const std::vector<size_t>& constantIndex)=0;

      virtual void adjust_shaders_for_view(GLContext* glContext,
					   const VecS& independentVariables,
					   const VecS& expressions,
					   double timeStamp)=0;

      static void set_light_direction(GLContext* glContext, const GLfloat* direction);
      static GLfloat* light_direction(GLContext* glContext);
	
    protected:
      Graph(GLWindow* glWindow, std::string identifier); 
      virtual ~Graph();


      GLWindow* glWindow_;
      std::string identifier_;
      
      double doubleClickTime_;
      double lastClickTime_;

      Marker::Type marker_;
      Marker::Type point_;

      GLfloat markerSize_;
      GLfloat pointSize_;

      GLfloat color_[4];

      GLfloat delay_;
      Style style_;
 
      /* for colors, an isoluminant color map that works both on white
	 and black background is hardcoded here */
      static const char* colorMapFunction_;

      static LightMap lightDirection_;

      /* this is the 1 point buffer for a dummy attribute, necessary because
	 we always need to enable vertex attribute array 0, at least on some 
	 implementations */
      static GLuint dummyBuffer_;

    private:
      Graph(const Graph&);
      Graph& operator=(const Graph&);

    };

  } /* end namespace gui */
} /* end namespace scigma */

#endif /* SCIGMA_GUI_GRAPH_HPP */
