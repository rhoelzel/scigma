  Wave varWave(2000);
  Wave constWave(1);
  
  for(size_t i=0;i<1000;++i)
    {
      varWave.push_back(double(i/10)/99.);
      varWave.push_back(double(i%10)/9.);
    }


  constWave.push_back(3.1415);

  Bundle b(&w,"bundle1",100,10,2,&varWave,&constWave);

  b.set_point_size(7);

GLfloat col[]={1,0,0,1}; 
b.set_color(col);

  std::vector<std::size_t> vBaseIndex,cIndex;
  vBaseIndex.push_back(0);
  vBaseIndex.push_back(1);
  cIndex.push_back(0);

  b.set_attributes_for_view(vBaseIndex,cIndex);

  
  std::vector<std::string> ind;
  std::vector<std::string> exp;
  
  ind.push_back("u");
  ind.push_back("v");
  ind.push_back("pi");

  /*exp.push_back("v");
    exp.push_back("u");*/

  exp.push_back("cos(2*pi*v)*sqrt(1-pow(u-1,2))");
  exp.push_back("sin(2*pi*v)*sqrt(1-pow(u-1,2))");

  exp.push_back("u-1");
  exp.push_back("0");
  exp.push_back("");

  b.adjust_shaders_for_view(w.gl_context(),ind,exp,0);

  w.gl_context()->add_drawable(&b);

b.set_style(Graph::POINTS);
Application::get_instance()->loop(5);
b.set_delay(0.15);
b.replay();
Application::get_instance()->loop(5);
b.set_style(Graph::ISOLINES);
Application::get_instance()->loop(5);
b.set_style(Graph::LINES);
Application::get_instance()->loop(150);
