#ifndef SCIGMA_GUI_BUNDLE_HPP
#define SCIGMA_GUI_BUNDLE_HPP

#include <string>
#include "../common/pythonobject.hpp"
#include "../dat/wave.hpp"
#include "glwindowevents.hpp"
#include "graph.hpp"
#include "drawable.hpp"
#include "glbuffer.hpp"

using scigma::common::PythonObject;
using scigma::common::EventSink;
using scigma::common::EventSource;
typedef scigma::dat::AbstractWave<double> Wave;
typedef scigma::gui::AbstractGLBuffer<GLfloat,double> GLfloatBuffer;

namespace scigma
{
  namespace gui
  {

    struct CurveClickEvent{
      typedef LOKI_TYPELIST_2(const char*,long) Arguments;};

    class GLWindow;

    class Bundle: public Graph,
		  public Drawable,
		  public PythonObject<Bundle>,
		  public EventSink<MouseButtonEvent>::Type,
		  public EventSource<CurveClickEvent>::Type
    {
      
    public:
      Bundle(GLWindow* glWindow, std::string identifier,
	     GLsizei length, GLsizei nRays, GLsizei nVars,
	     Wave* varyings, Wave* constants);
      ~Bundle();

      bool process(MouseButtonEvent event, GLWindow* w, int button , int action, int mods);
      
      // methods from the Drawable interface 
      static void on_gl_context_creation(GLContext* glContext);
      static void on_gl_context_destruction(GLContext* glContext);
      static void before_batch_draw(GLContext* glContext);
      void on_addition(GLContext* context);
      void on_removal(GLContext* context);
      void draw(GLContext* glContext);
      void on_hover_begin(GLContext* glContext);
      void on_hover(GLContext* glContext, GLuint value);
      void on_hover_end(GLContext* glContext);
      
      // virtual methods inherited from Graph
      virtual void replay();
      virtual void finalize();

      virtual void set_attributes_for_view(const std::vector<size_t>& varyingBaseIndex,
					   const std::vector<size_t>& constantIndex);

      virtual void adjust_shaders_for_view(GLContext* glContext,
					   const VecS& independentVariables,
					   const VecS& expressions,
					   double timeStamp);
      virtual void set_style(Style style);
      
    private:
      void prepare_varying_attributes();
      void prepare_constant_attributes();
      
      static const std::string vertexShaderGL2_;
      static const std::string vertexShaderGL3_;
      static const std::string fragmentShaderGL2_;
      static const std::string fragmentShaderGL3_;

      static std::map<GLContext*,GLint> nDrawnLocationMap_;
      static std::map<GLContext*,GLint> nTotalLocationMap_;
      static std::map<GLContext*,GLint> spriteLocationMap_;
      static std::map<GLContext*,GLint> sizeLocationMap_;
      static std::map<GLContext*,GLint> colorLocationMap_;
      static std::map<GLContext*,GLint> lighterLocationMap_;

      static GLint nDrawnLocation_;
      static GLint nTotalLocation_;
      static GLint spriteLocation_;
      static GLint sizeLocation_;
      static GLint colorLocation_;
      static GLint lighterLocation_;

      static GLuint program_;
      static double shaderTimeStamp_;
      
      GLsizei length_;
      GLsizei nRays_;
      GLsizei nVars_;
      GLsizei nConsts_;
      
      GLuint isoIndexBuffer_;
      GLuint isoIndexAttributeBuffer_; 
      GLuint rayIndexBuffer_;
      GLuint rayIndexAttributeBuffer_;
      
      GLfloatBuffer varyingBuffer_;
      std::vector<double> constants_;

      std::vector<size_t> varyingBaseIndex_;
      std::vector<size_t> constantIndex_;

      GLsizei last_;
      GLsizei pickPoint_;
      
      bool varyingAttributesInvalid_;
      bool hovering_;
      bool picking_;
      
      char padding_[5];
    };
    
  } /* end namespace gui */
} /* end namespace scigma */

#endif /* SCIGMA_GUI_BUNDLE_HPP */
