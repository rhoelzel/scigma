#include "graph.hpp"

namespace scigma
{
  namespace gui
  {

    const char* Graph::colorMapFunction_=
      "vec4 colormap_(in float p)\n"
      "{\n"
      "vec4 color;\n"
      "\tif(p<0.0||p>1.0)\n"
      "\t\tcolor= vec4(1,0.0,1,1.0);\n"
      "\telse if (p<0.5)\n"
      "\t\tcolor= mix(vec4(0,0,1.0,1.0),vec4(0.0,1.0,0,1.0),p*2);\n"
      "\telse\n"
      "\t\tcolor= mix(vec4(0,1.0,0.0,1.0),vec4(1.0,0.0,0.0,1.0),p*2-1);\n"
      "\treturn color\n;"
      "}\n\n";
    //+*/mix(vec4(0,1.0,0.0,1.0),vec4(1.0,0.0,0,1.0),p*2-1)*/*step(p,0.5)*/;\n"
    
    
    /*      "vec4 colormap_(in float p)\n"
	    "{\n"
	    "\tif(p<0.0||p>1.0)\n"
	    "\t\treturn vec4(0.72,0.0,0.72,1.0);\n"
	    "\treturn vec4(\n"
	    "\t\tclamp(-0.95*p*p+0.86*p+0.43+2.2*abs(p-0.35),0,1),\n"
	    "\t\t-1.1*p*p+0.58*p+0.75-0.56*abs(p-0.75),\n"
	    "\t\t-0.32*p*p+0.64*p+0.4-1.2*abs(p-0.5),\n"
	    "\t\t1.0);\n"
	    "}\n\n";*/

    GLuint Graph::dummyBuffer_(0);
    
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wglobal-constructors"
 #pragma clang diagnostic ignored "-Wexit-time-destructors"
    Graph::LightMap Graph::lightDirection_;
 #pragma clang diagnostic pop

    
    Graph::Graph(GLWindow* glWindow, std::string identifier):
      glWindow_(glWindow),identifier_(identifier),doubleClickTime_(0.25),lastClickTime_(-1.0),
      marker_(Marker::STAR),point_(Marker::DOT),
      markerSize_(16),pointSize_(1),
      delay_(0),style_(POINTS)
    {
      if(!dummyBuffer_)
	{
	  glGenBuffers(1,&dummyBuffer_);
	  glBindBuffer(GL_ARRAY_BUFFER,dummyBuffer_);
	  glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(sizeof(GLfloat)*1),NULL,GL_DYNAMIC_DRAW);
	  glBindBuffer(GL_ARRAY_BUFFER,0);
	}
      
      GLfloat col[]={1.0f,1.0f,1.0f,1.0f};
      set_color(col);
    }
    
    Graph::~Graph()
    {}
    
    void Graph::set_marker_style(Marker::Type marker){marker_=marker;}
    Marker::Type Graph::marker_style() const {return marker_;}

    void Graph::set_marker_size(GLfloat size){markerSize_=size<1.0f?1.0f:size;}
    GLfloat Graph::marker_size() const{return markerSize_;}
    
    void Graph::set_point_style(Marker::Type marker){point_=marker;}
    Marker::Type Graph::point_style() const{return point_;}
    
    void Graph::set_point_size(GLfloat size){pointSize_=size<1.0f?1.0f:size;}
    GLfloat Graph::point_size() const{return pointSize_;}
    
    void Graph::set_color(const GLfloat* color)
    {
      for(size_t i(0);i<4;++i)
	color_[i]=color[i];
    }
    const GLfloat* Graph::color() const{return color_;}

    void Graph::set_delay(GLfloat delay){delay_=delay<0.0?0.0:delay_;}
    GLfloat Graph::delay() const{return delay_;}

    void Graph::set_style(Style style){style_=style;}
    GLfloat Graph::style() const{return style_;}

    void Graph::set_light_direction(GLContext* glContext, const GLfloat* direction)
    {
      std::vector<GLfloat>& dir(lightDirection_[glContext]);
      dir.resize(3);
      for(size_t i(0);i<3;++i)
	dir[i]=direction[i];
    }

    GLfloat* Graph::light_direction(GLContext* glContext)
    {
      std::vector<GLfloat>& dir(lightDirection_[glContext]);
      if(dir.size())
	return &dir[0];
      return NULL;
    }
        
  } /* end namespace gui */
} /* end namespace scigma */
