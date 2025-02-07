build:
	@docker build . -t tcc_ufrr

hello:
	@docker run -it tcc_ufrr "--run=hello_simulator"

sim:
	docker run -ti --rm -v tcc_ufrr_logs:/app/logs tcc_ufrr --run "sim --traceFile=scratch/ns3.tcl --nodeNum=1 --duration=100.0 --logFile=/app/logs/sim.log"


.PHONY: build hello sim
