all: clean configure staticlib

clean:
	rm -rf ./bin

configure:
	mkdir -p ./bin
	cd ./bin && cmake .. -DCMAKE_BUILD_TYPE=Release

staticlib:
	cd ./bin && make pmemds pmemds-client

test: configure reset
