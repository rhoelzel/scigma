#include "../log.hpp"
#include <algorithm>
#include <vector>
#include <string>
#include <catch.hpp>
#include <tinythread.h>

using scigma::common::Log;

SCENARIO("Log: writing messages with different flags in a single thread","[Log][single-thread]")
{
  GIVEN("A Log object and a number of messages")
    {
      Log log("test.log");
      std::string message1("this is a test!");
      std::string message2("this is a another test!");
      std::string error1("this is an error!");
      std::string error2("this is another error!");
      std::string warning1("this a warning!");
      std::string warning2("this another warning!");

      WHEN("empty messages are passed")
	{
	  log.push("");
	  log.push(message1);
	  THEN("they are ignored")
	    REQUIRE(log.pop()==(std::pair<Log::Type,std::string>(Log::DEFAULT,message1)));
	}
      WHEN("The messages are passed with different flags push<Log::Type>(...)")
	{
	  log.push(message1);
	  log.push<Log::ERROR>(error1);
	  log.push<Log::WARNING>(warning1);
	  log.push<Log::SUCCESS>(message1);
	  log.push<Log::SUCCESS>(message2);
	  log.push<Log::FAIL>(message1);
	  log.push<Log::FAIL>(message2);
	  log.push(message2);
	  log.push<Log::ERROR>(error2);
	  log.push<Log::WARNING>(warning2);
	  log.push<Log::DATA>(message1);
	  log.push<Log::DATA>(message2);
	  
	  THEN("The messages are retrieved correctly with pop<Log::Type>(), in the order"
	       "they were submitted per Log::Type")
	    {
	      /*  REQUIRE(log.pop()==message1);
	      REQUIRE(log.pop()==message2);
	      REQUIRE(log.pop()=="");
	      REQUIRE(log.pop<Log::SUCCESS>()==message1);
	      REQUIRE(log.pop<Log::SUCCESS>()==message2);
	      REQUIRE(log.pop<Log::SUCCESS>()=="");
	      REQUIRE(log.pop<Log::FAIL>()==message1);
	      REQUIRE(log.pop<Log::FAIL>()==message2);
	      REQUIRE(log.pop<Log::FAIL>()=="");
	      REQUIRE(log.pop<Log::ERROR>()==error1);
	      REQUIRE(log.pop<Log::ERROR>()==error2);
	      REQUIRE(log.pop<Log::ERROR>()=="");
	      REQUIRE(log.pop<Log::WARNING>()==warning1);
	      REQUIRE(log.pop<Log::WARNING>()==warning2);
	      REQUIRE(log.pop<Log::WARNING>()=="");
	      REQUIRE(log.pop<Log::DATA>()==message1);
	      REQUIRE(log.pop<Log::DATA>()==message2);
	      REQUIRE(log.pop<Log::DATA>()=="");*/
	    }
	}
    }
}

std::vector<std::string> output;
tthread::mutex mutex;

bool push_back(std::string s)
{
  tthread::lock_guard<tthread::mutex> guard(mutex);
  if(output.size()==6*95*2)
    return true;
  if(s!="")
    output.push_back(s);
  return false;
}

void read(void* data)
{
  Log* log = static_cast<Log*>(data);
  /*
  while(true)
    {
      if(push_back(log->pop<Log::SUCCESS>()))return;
      if(push_back(log->pop<Log::FAIL>()))return;
      if(push_back(log->pop<Log::ERROR>()))return;
      if(push_back(log->pop<Log::WARNING>()))return;
      if(push_back(log->pop<Log::DATA>()))return;
      if(push_back(log->pop()))return;
      }*/
}

void write1(void* data)
{
  Log* log = static_cast<Log*>(data);
  for(char j(32);j<127;++j)
    {
      log->push<Log::SUCCESS>(std::string("test")+j);
      log->push<Log::FAIL>(std::string("test")+j);
      log->push<Log::ERROR>(std::string("test")+j);
      log->push<Log::WARNING>(std::string("test")+j);
      log->push<Log::DATA>(std::string("test")+j);
      log->push(std::string("test")+j);
    }
}

void write2(void* data)
{
  Log* log = static_cast<Log*>(data);
  for(char j(32);j<127;++j)
    {
      log->push<Log::SUCCESS>(std::string("fest")+j);
      log->push<Log::FAIL>(std::string("fest")+j);
      log->push<Log::ERROR>(std::string("fest")+j);
      log->push<Log::WARNING>(std::string("fest")+j);
      log->push<Log::DATA>(std::string("fest")+j);
      log->push(std::string("fest")+j);
    }
}

SCENARIO("Log: writing messages in multiple threads","[Log][multi-thread]")
{
  GIVEN("A Log that is written to by two threads and read from by two other threads")
    {
      Log log;
      tthread::thread thread1(write1,static_cast<void*>(&log));
      tthread::thread thread2(write2,static_cast<void*>(&log));
      tthread::thread thread3(read,static_cast<void*>(&log));
      tthread::thread thread4(read,static_cast<void*>(&log));

      thread1.join();
      thread2.join();
      thread3.join();
      thread4.join();

   
      THEN("All messages are retrieved correctly without deadlocks")
	{
	  std::sort(output.begin(),output.end());
	  size_t count(0);
	  for(char j(32);j<127;++j)
	    for(size_t i(0);i<6;++i)
	      REQUIRE(output[count++]==std::string("fest")+j);
	  for(char j(32);j<127;++j)
	    for(size_t i(0);i<6;++i)
	      REQUIRE(output[count++]==std::string("test")+j);
	}
    }
}
