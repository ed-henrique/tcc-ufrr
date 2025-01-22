# TCC da UFRR

Repositório do meu protótipo do TCC da UFRR.

## Descrição

Protótipo de um dispositivo rastreador, que coleta constantemente dados de localização mesmo em
áreas de baixa conectividade.

## Big Picture

<!-- Em Progresso -->

## Como Funciona

A localização é coletada via protocolo NB-IoT na rede 5G, quando disponível, e via GPS, quando não
disponível. Ao coletar a localização, ela é armazenada em banco de dados local. Há uma tentativa de
envio desse dado para o banco de dados em nuvem, que se falha, toma nota da identificação do último
ponto de sincronização para tentar novamente em outro momento.

Isso se repete constantemente, de forma que mesmo quando o objeto estiver fora de rede, sua
localização ainda é coletada para uma sincronização posterior.

## Desafios

- [ ] Coletar dados de localização via NB-IoT na rede 5G
- [ ] Coletar dados de localização via GPS
- [ ] Armazenar dados coletados localmente
- [ ] Estabelecer conexão com banco de dados em nuvem para envio dos dados (considerando que a
conexão não é contínua)
- [ ] Garantir a persistência dos dados em nuvem
- [ ] Fornecer interface para consulta dos dados
- [ ] Comparar o consumo de energia do uso do NB-IoT e do GPS
- [ ] Coletar métricas de consumo de recursos do dispositivo durante toda a execução do protótipo

### Desafios Adicionais

- [ ] Garantir que os dados não foram alterados localmente antes da sincronização

## Imagens

<!-- Em Progresso -->

## Paper

<!-- Em Progresso -->

## Referências

<!-- Em Progresso -->
