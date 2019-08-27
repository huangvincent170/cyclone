all: clean configure

clean:
	rm -rf ./bin

configure:
	mkdir -p ./bin
	cd ./bin && cmake ..

server: configure
	cd ./bin && make pmemds

client: configure
	cd ./bin && make pmemds-client

test: configure 
