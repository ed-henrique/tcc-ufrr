build:
	docker build . -t tcc_ufrr

hello:
	docker run -it tcc_ufrr "--run=hello_simulator"
