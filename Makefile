build.nb_iot:
	@docker build -t tcc_ufrr_nb_iot -f Dockerfile.NB-IoT .

run.nb_iot:
	@docker build -t tcc_ufrr_nb_iot -f Dockerfile.NB-IoT .

# Older NB-IoT implementation, not as many articles about it
build.nb_iot_2:
	@docker build -t tcc_ufrr_nb_iot_2 -f Dockerfile.NB-IoT-2 .

# Older NB-IoT implementation, not as many articles about it
run.nb_iot_2:
	@docker build -t tcc_ufrr_nb_iot_2 -f Dockerfile.NB-IoT-2 .

build.lorawan:
	@docker build -t tcc_ufrr_lorawan -f Dockerfile.LoRaWAN .

run.lorawan:
	@docker build -t tcc_ufrr_lorawan -f Dockerfile.LoRaWAN .

build.sigfox:
	@docker build -t tcc_ufrr_sigfox -f Dockerfile.Sigfox .

run.sigfox:
	@docker build -t tcc_ufrr_sigfox -f Dockerfile.Sigfox .

.PHONY: build.nb_iot build.nb_iot_2 build.lorawan build.sigfox run.nb_iot run.nb_iot_2 run.lorawan run.sigfox
