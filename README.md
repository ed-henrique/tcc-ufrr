# TCC da UFRR

Repositório da minha simulação do TCC da UFRR.

## Descrição

Simulação, por meio do [ns-3](https://www.nsnam.org/), de um dispositivo rastreador, que coleta
constantemente dados de localização mesmo em áreas de baixa conectividade, utilizando o protocolo
NB-IoT em áreas de cobertura 5G.

## Big Picture

<!-- Em Progresso -->

## Como Funciona

Na simulação, a localização é coletada via protocolo NB-IoT na rede 5G, quando disponível, e via
GPS, quando não disponível. Ao coletar a localização, ela é armazenada em banco de dados local. Há
uma tentativa de envio desse dado para o banco de dados em nuvem, que se falha, toma nota da
identificação do último ponto de sincronização para tentar novamente em outro momento.

Isso se repete constantemente, de forma que mesmo quando o objeto estiver fora de rede, sua
localização ainda é coletada para uma sincronização posterior.

## Desafios

- [ ] Simular a coleta de dados de localização via NB-IoT na rede 5G
- [ ] Simular a coleta de dados de localização via GPS
- [ ] Simular o armazenamento dos dados coletados localmente
- [ ] Simular a conexão com o banco de dados em nuvem para envio dos dados (considerando que a
conexão não é contínua)
- [ ] Garantir a persistência dos dados em nuvem
- [ ] Fornecer interface para consulta dos dados
- [ ] Comparar os consumos de energia das abordagens usando o NB-IoT e o GPS
- [ ] Coletar as métricas de consumo de recursos pelo simulador durante toda a sua execução

### Desafios Adicionais

- [ ] Garantir que os dados não foram alterados localmente antes da sincronização

## Imagens

<!-- Em Progresso -->

## Paper

<!-- Em Progresso -->

## Referências

<!-- Em Progresso -->
