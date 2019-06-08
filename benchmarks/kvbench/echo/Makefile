include ../../Makefile.inc

server: echo_server
client: echo_async_client

echo_server:echo_server.cpp
	$(CXX) $(SERVER_CXXFLAGS) echo_server.cpp $(BOOST_THREAD_LIB) $(SERVER_LIBS) -o $@

echo_client:echo_client.cpp
	$(CXX) $(CLIENT_CXXFLAGS) echo_client.cpp $(BOOST_THREAD_LIB) $(CLIENT_LIBS) -o $@

echo_async_client:echo_async_client.cpp
	$(CXX) $(CLIENT_CXXFLAGS) echo_async_client.cpp $(BOOST_THREAD_LIB) $(CLIENT_LIBS) -o $@


.PHONY:clean

clean:
	rm -f echo_server echo_client echo_async_client
