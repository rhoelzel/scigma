#include "../common/util.hpp"
#include "sheet.hpp"
#include "sheetshaders.hpp"
#include "glcontext.hpp"
#include "glwindow.hpp"
#include <vector>
#include <iostream>

using scigma::common::connect_before;
using scigma::common::disconnect;

namespace scigma
{
  namespace gui
  {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
    std::map<GLContext*,GLint> Sheet::spriteLocationMap_;
    std::map<GLContext*,GLint> Sheet::sizeLocationMap_;
    std::map<GLContext*,GLint> Sheet::colorLocationMap_;
    std::map<GLContext*,GLint> Sheet::lighterLocationMap_;
    std::map<GLContext*,GLint> Sheet::lightDirLocationMap_;
    std::map<GLContext*,GLint> Sheet::lightParamLocationMap_;
#pragma clang diagnostic pop

    GLint Sheet::spriteLocation_;
    GLint Sheet::sizeLocation_;
    GLint Sheet::colorLocation_;
    GLint Sheet::lighterLocation_;
    GLint Sheet::lightDirLocation_;
    GLint Sheet::lightParamLocation_;

    GLuint Sheet::program_(0);
    double Sheet::shaderTimeStamp_(-1);

    Sheet::Sheet(GLWindow* glWindow, std::string identifier,
		 const Mesh* mesh, GLsizei nVars, const Wave* constants):
      Graph(glWindow,identifier),
      PythonObject<Sheet>(this),
      nVars_(nVars), nConsts_(GLsizei(constants->size())),
      mesh_(mesh),
      isoIndexBuffer_(&mesh->iso_indices()),
      isoEndPointsBuffer_(&mesh->iso_end_points()),
      triangleIndexBuffer_(&mesh->triangle_indices()),
      varyingBuffer_(&mesh->triangle_data()),
      /*last_(-1),*/pickPoint_(-1),
      varyingAttributesInvalid_(true),
      hovering_(false),
      picking_(false)
    {
      constants->lock();
      for(size_t i(0), size(constants->size()*Mesh::NVALS_PER_DIM);i<size;++i)
	constants_.push_back(GLfloat(constants->data()[i/Mesh::NVALS_PER_DIM]));
      constants->unlock();

      isoIndexBuffer_.begin_transfer();
      isoEndPointsBuffer_.begin_transfer();
      triangleIndexBuffer_.begin_transfer();
      varyingBuffer_.begin_transfer();

      GLfloat dir[]={1.0f,1.0f,1.0f,1.0f};
      set_light_direction(dir);

      GLfloat ambient(0.4f);
      GLfloat diffuse(0.6f);
      GLfloat specular(0.07f);
      GLfloat highlight(0.8f);

      GLfloat param[]={ambient,diffuse,specular,highlight};
      set_light_parameters(param);
      
      padding_[0]=0;
    }

    Sheet::~Sheet()
    {

    }

    void Sheet::replay()
    {

    }
    
    void Sheet::finalize()
    {
      isoIndexBuffer_.end_transfer();
      isoEndPointsBuffer_.end_transfer();
      triangleIndexBuffer_.end_transfer();
      varyingBuffer_.end_transfer();
    }

    void Sheet::on_gl_context_creation(GLContext* glContext)
    {
      /* this is just a dummy program which will be replaced in
	 adjust_shaders_for_view()
      */
      glContext->create_programs<Sheet>(1);
      GLuint program; glContext->get_programs<Sheet>(&program);
      const char* vertexShader="void main(){gl_Position=rotationMatrix*vec4(1,1,1,1);}";
      #ifdef SCIGMA_USE_OPENGL_3_2
      const char* fragmentShader="out vec4 color;void main(){color=vec4(1,1,1,1);}";
      #else
      const char* fragmentShader="void main(){gl_FragColor=vec4(1,1,1,1);}";
      #endif
      glContext->compile_program(program,vertexShader,"",fragmentShader);
      glContext->link_program(program);
      GLERR;
    }

    void Sheet::on_gl_context_destruction(GLContext* glContext)
    {
      glContext->delete_programs<Sheet>();
      GLERR;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

    bool Sheet::process(MouseButtonEvent event, GLWindow* w, int button , int action, int mods)
    {
      if(GLFW_MOUSE_BUTTON_LEFT==button&&GLFW_PRESS==action)
	{
	  double time(glfwGetTime());
	  double dt(time-lastClickTime_);
	  lastClickTime_=time;
	  if(dt>doubleClickTime_)
	    return true;
	  EventSource<PointClickEvent>::Type::emit(identifier_.c_str(),int(pickPoint_));
	  return true;
	}
      return false;
    }

    void Sheet::on_addition(GLContext* glContext)
    {
      glContext->request_redraw();
    }

    void Sheet::on_removal(GLContext* glContext)
    {
      disconnect<MouseButtonEvent>(glWindow_,this);
      glContext->request_redraw();
    }
    
    void Sheet::before_batch_draw(GLContext* glContext)
    {
      glContext->get_programs<Sheet>(&program_);
      glUseProgram(program_);
      spriteLocation_=spriteLocationMap_[glContext];
      colorLocation_=colorLocationMap_[glContext];
      sizeLocation_=sizeLocationMap_[glContext];
      lighterLocation_=lighterLocationMap_[glContext];
      lightDirLocation_=lightDirLocationMap_[glContext];
      lightParamLocation_=lightParamLocationMap_[glContext];
      
      glActiveTexture(GL_TEXTURE0);
      glEnable(GL_DEPTH_TEST);
      glDisable(GL_BLEND);
      GLERR;
    }
    
    
    void Sheet::set_attributes_for_view(const std::vector<size_t>& varyingBaseIndex,
					 const std::vector<size_t>& constantIndex)
    {
      varyingBaseIndex_=varyingBaseIndex;
      constantIndex_=constantIndex;
      varyingAttributesInvalid_=true;
    }
    
    using scigma::common::substring;
    
    void Sheet::adjust_shaders_for_view(GLContext* glContext,
					const VecS& independentVariables,
					const VecS& expressions,
					double timeStamp)
    {
      if(timeStamp<shaderTimeStamp_)
	return;
      shaderTimeStamp_=timeStamp;
      
#ifdef SCIGMA_USE_OPENGL_3_2
      std::string vShader(vertexShaderGL3_);
      std::string fShader(fragmentShaderGL3_);
      std::string vertexIn("in");
#else
      std::string vShader(vertexShaderGL2_);
      std::string fShader(fragmentShaderGL2_);
      std::string vertexIn("attribute");
#endif
      std::string attributes,pos0,pos1,pos2,pos3;
      for(VecS::const_iterator i(independentVariables.begin()), end(independentVariables.end());i!=end;++i)
	{
	  attributes+=vertexIn+" vec4 "+*i+"_attribute_;\n";
	  pos0+="float "+*i+" = "+*i+"_attribute_.x;";
	  pos1+=*i+" = "+*i+"_attribute_.y;";
	  pos2+=*i+" = "+*i+"_attribute_.z;";
	  pos3+=*i+" = "+*i+"_attribute_.w;";
	}
      pos0+="\n";pos1+="\n";pos2+="\n";pos3+="\n";
      substring(vShader,"__REPLACE_ATTRIBUTES__",attributes);
      substring(vShader,"__REPLACE_POS_0__",pos0);
      substring(vShader,"__REPLACE_POS_1__",pos1);
      substring(vShader,"__REPLACE_POS_2__",pos2);
      substring(vShader,"__REPLACE_POS_3__",pos3);
      substring(vShader,"__REPLACE_X__",expressions[0]);
      substring(vShader,"__REPLACE_Y__",expressions[1]);
      substring(vShader,"__REPLACE_Z__",expressions[2]);
      substring(vShader,"__REPLACE_TIME__",expressions[3]);

      /* color: defined by uniform or by expression? */
      if(""==expressions[4])
	{
	  substring(vShader,"__REPLACE_COLOR_START__","/*");
	  substring(vShader,"__REPLACE_COLOR_END__","*/");
	  substring(fShader,"__REPLACE_COLOR_UNIFORM_START__","");
	  substring(fShader,"__REPLACE_COLOR_UNIFORM_END__","");
	  substring(fShader,"__REPLACE_COLOR_IN_START__","/*");
	  substring(fShader,"__REPLACE_COLOR_IN_END__","*/");
	}
      else
	{
	  substring(vShader,"__REPLACE_COLOR_START__","");
	  substring(vShader,"__REPLACE_COLOR_END__","");
	  substring(vShader,"__REPLACE_COLOR__",expressions[4]);
	  substring(fShader,"__REPLACE_COLOR_UNIFORM_START__","/*");
	  substring(fShader,"__REPLACE_COLOR_UNIFORM_END__","*/");
	  substring(fShader,"__REPLACE_COLOR_IN_START__","");
	  substring(fShader,"__REPLACE_COLOR_IN_END__","");
	}

      std::cout<<"------------------------------------------------------------------"<<std::endl;
      std::cout<<vShader;
      std::cout<<"------------------------------------------------------------------"<<std::endl;
      std::cout<<fShader;

      
      glContext->delete_programs<Sheet>();
      glContext->create_programs<Sheet>(1);
      glContext->get_programs<Sheet>(&program_);

      glBindAttribLocation(program_,0,"index_");
      glBindAttribLocation(program_,1,"endPoint_");
      for(size_t i(0),size(independentVariables.size());i<size;++i)
	glBindAttribLocation(program_,GLuint(i+2),(independentVariables[i]+"_attribute_").c_str());
            
      glContext->compile_program(program_,vShader,"",fShader);
      glContext->link_program(program_);

      /* if color is defined by uniform, get the location */
      if(""==expressions[4])
	colorLocationMap_[glContext]=glGetUniformLocation(program_,"rgba_");
      else
	colorLocationMap_[glContext]=-1;

      GLERR;

      spriteLocationMap_[glContext]=glGetUniformLocation(program_,"sprite_");
      sizeLocationMap_[glContext]=glGetUniformLocation(program_,"size_");
      lighterLocationMap_[glContext]=glGetUniformLocation(program_,"lighter_");
      /*      lightDirLocationMap_[glContext]=glGetUniformLocation(program_,"lightDir_");
	      lightParamTotalLocationMap_[glContext]=glGetUniformLocation(program_,"lightParam_");*/

      glUseProgram(program_);
      glUniform1i(glGetUniformLocation(program_,"sampler_"),0);
      
      GLERR;

      varyingAttributesInvalid_=true;
    }

    void Sheet::set_style(Style style)
    {
      if(style!=style_)
	{
	  this->Graph::set_style(style);
	  varyingAttributesInvalid_=true;
	}
    }

    void Sheet::set_light_direction(GLfloat* direction)
    {
      for(size_t i(0);i<4;++i)
	lightDirection_[i]=direction[i];
    }

    void Sheet::set_light_parameters(GLfloat* parameter)
    {
      for(size_t i(0);i<4;++i)
	lightParameter_[i]=parameter[i];
    }

    void Sheet::draw(GLContext* glContext)
    {
      /* setup vertex attributes (if necessary) */
#ifdef SCIGMA_USE_OPENGL_3_2
      glBindVertexArray(vertexArray_);
      if(varyingAttributesInvalid_)
	prepare_varying_attributes();
#else
      prepare_varying_attributes();
#endif
      prepare_constant_attributes();
      GLERR;
      /* set the color, if we do not use a color map */
      if(-1!=colorLocation_)
	glUniform4fv(colorLocation_,1,color_);
      std::cout<<"colorLocation:"<<colorLocation_<<", ";
      GLERR;
      /* display the bundle in a lighter color, if hovering */
      if(hovering_&&!picking_)
	glUniform1i(lighterLocation_,1);
      else
	glUniform1i(lighterLocation_,0);
      std::cout<<"lighterLocation:"<<lighterLocation_<<", ";
      GLERR;

      switch(style_)
	{
	case Graph::WIREFRAME:
	  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	  draw_triangles();
	  break;
	case Graph::SOLID:
	  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	  draw_triangles();
	  break;
	case Graph::LINES:
	  draw_isolines();
	  break;
	case Graph::ISOLINES:
	  draw_isolines();
	  break;
	case Graph::POINTS:
	  draw_points();
	  break;
	}
    }

    void Sheet::on_hover_begin(GLContext* glContext)
    {
    }
    
    void Sheet::on_hover(GLContext* glContext, GLuint value)
    {
      //glfwGetKey
    }

    void Sheet::on_hover_end(GLContext* glContext)
    {
    }
  
#pragma GCC diagnostic pop

    void Sheet::prepare_varying_attributes()
    {
      /* bind buffer with triangle indexes in to first attribute */
      glBindBuffer(GL_ARRAY_BUFFER,triangleIndexBuffer_.buffer_ID());
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0,1,GL_UNSIGNED_INT,GL_FALSE,0,0);
      GLERR;
      /* bind buffer for all varying values and set respective attributes */
      glBindBuffer(GL_ARRAY_BUFFER,varyingBuffer_.buffer_ID());
      size_t nVaryingAttributes(varyingBaseIndex_.size());
      for(size_t i(0);i<nVaryingAttributes;++i)
	{
	  GLERR;
	  GLuint attLoc((GLuint(i+2)));
	  glEnableVertexAttribArray(attLoc);
	  glVertexAttribPointer(attLoc,4,GL_FLOAT, GL_FALSE,
				nVars_*GLsizei(Mesh::NVALS_PER_DIM*sizeof(GLfloat)), 
				reinterpret_cast<const GLvoid*>(sizeof(GLfloat)*varyingBaseIndex_[i]*Mesh::NVALS_PER_DIM));
      GLERR;
      std::cout<<attLoc<<std::endl;
	}
      varyingAttributesInvalid_=false;
    }

    void Sheet::prepare_constant_attributes()
    {
      GLERR;
      size_t nVaryingAttributes(varyingBaseIndex_.size());
      size_t nConstAttributes(constantIndex_.size());
      for(size_t i(0);i<nConstAttributes;++i)
	{
	  GLuint attLoc((GLuint(i+nVaryingAttributes+2)));
	  glDisableVertexAttribArray(attLoc);
	  glVertexAttrib4fv(attLoc,&constants_[i*Mesh::NVALS_PER_DIM]);
      GLERR;
            std::cout<<attLoc<<std::endl;
	}
    }

    void Sheet::draw_triangles()
    {
      /* check how many layers are available for drawing */
      size_t availableLayer(mesh_->available_triangle_layer(triangleIndexBuffer_.size(),varyingBuffer_.size()));
      GLsizei maxIndex((GLsizei(mesh_->max_for_triangle_layer(availableLayer))));

      glDisableVertexAttribArray(1);
      glVertexAttrib1f(1,0.f);
      std::cout<<"availableLayers: "<<availableLayer<<"; maxIndex: "<<maxIndex<<std::endl;
      GLERR;
      /* draw line strip */
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,triangleIndexBuffer_.buffer_ID());
      //glBindBuffer(GL_ARRAY_BUFFER,varyingBuffer_.buffer_ID());
      glDrawElements(GL_TRIANGLE_STRIP,maxIndex,GL_UNSIGNED_INT,reinterpret_cast<const void*>(0));
      GLERR;

    }
      
    void Sheet::draw_isolines()
    {
      /* check how many layers are available for drawing */
      size_t availableLayer(mesh_->available_iso_layer(isoIndexBuffer_.size(),
						       isoEndPointsBuffer_.size(),
						       varyingBuffer_.size()));
      GLsizei maxIndex((GLsizei(mesh_->max_for_iso_layer(availableLayer))));

      /* draw line strip */
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,isoIndexBuffer_.buffer_ID());
      glBindBuffer(GL_ARRAY_BUFFER,isoEndPointsBuffer_.buffer_ID());
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1,1,GL_BYTE,GL_FALSE,0,0);

      //      glBindBuffer(GL_ARRAY_BUFFER,varyingBuffer_.buffer_ID());
      glDrawElements(GL_LINE_STRIP,maxIndex,GL_UNSIGNED_INT,reinterpret_cast<const void*>(0));
      GLERR;
    }

    
    void Sheet::draw_points()
    {
      /* check how many layers are available for drawing */
      size_t availableLayer(mesh_->available_iso_layer(isoIndexBuffer_.size(),
						       isoEndPointsBuffer_.size(),
						       varyingBuffer_.size()));
      GLsizei maxIndex((GLsizei(mesh_->max_for_iso_layer(availableLayer))));

      /* draw line strip */
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,isoIndexBuffer_.buffer_ID());
      glDisableVertexAttribArray(1);
      glVertexAttrib1f(1,0.f);

      GLfloat factor(1.0f);
      if(hovering_&&!picking_)
	factor=1.2f;
      glBindTexture(GL_TEXTURE_2D,Marker::texture_id(point_));
      glUniform1i(spriteLocation_,1);
      GLERR;
      glPointSize(pointSize_*factor);
      GLERR;
      glUniform1f(sizeLocation_,pointSize_*factor);
      //      glBindBuffer(GL_ARRAY_BUFFER,varyingBuffer_.buffer_ID());
      glDrawElements(GL_POINTS,maxIndex,GL_UNSIGNED_INT,reinterpret_cast<const void*>(0));
      GLERR;

    }

    

  } /* end namespace gui */
} /* end namespace scigma */

