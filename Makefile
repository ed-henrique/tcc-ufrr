build.nb_iot:
	@docker build -t tcc_ufrr_nb_iot -f Dockerfile.NB-IoT .

run.nb_iot:
	@docker run -it --rm -v ./logs/:/logs/ tcc_ufrr_nb_iot --run "nb_iot"

# Older NB-IoT implementation, not as many articles about it
build.nb_iot_2:
	@docker build -t tcc_ufrr_nb_iot_2 -f Dockerfile.NB-IoT-2 .

# Older NB-IoT implementation, not as many articles about it
run.nb_iot_2:
	@docker build -t tcc_ufrr_nb_iot_2 -f Dockerfile.NB-IoT-2 .

build.lorawan:
	@docker build -t tcc_ufrr_lorawan -f Dockerfile.LoRaWAN .

run.lorawan:
	@docker run -it --rm -v ./logs/:/usr/ns3/ns-3-dev/logs/ tcc_ufrr_lorawan run "lorawan"

build.sigfox:
	@docker build -t tcc_ufrr_sigfox -f Dockerfile.Sigfox .

run.sigfox:
	@docker run -it --rm -v ./logs/:/usr/ns3/ns-3-dev/logs/ tcc_ufrr_sigfox --run "sigfox"

build.wifi:
	@docker build -t tcc_ufrr_wifi -f Dockerfile.WiFi .

run.wifi:
	@docker run -it --rm -v ./logs/:/usr/ns3/ns-3-dev/logs/ tcc_ufrr_wifi run "wifi"

.PHONY: build.nb_iot build.nb_iot_2 build.lorawan build.sigfox build.wifi run.nb_iot run.nb_iot_2 run.lorawan run.sigfox run.wifi
