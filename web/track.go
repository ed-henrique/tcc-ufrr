package main

import "time"

// TrackInfo armazena onde e quando um objeto esteve.
type TrackInfo struct {
	Time time.Time
	Lat  float64
	Lon  float64
}

// Trackeable permite maior flexibilidade para diferentes tipos de objeto rastreáveis.
type Trackeable interface {
	SyncTrackingHistory([]TrackInfo)
	GetLastLocation() TrackInfo
	GetTrackingHistory() []TrackInfo
}

// Object é identificado por um ID e contém um histórico de rastreamento.
type Object struct {
	ID        int64
	trackInfo []TrackInfo
}

func (o *Object) SyncTrackingHistory(trackInfo []TrackInfo) {
	o.trackInfo = append(o.trackInfo, trackInfo...)
}

func (o *Object) GetLastLocation() TrackInfo {
	// TODO: Verificar se esse objeto deve ser passado por valor ou referência.
	// TODO: Verificar se o que deve ser retornado é um erro ou um TrackInfo padrão quando não houver
	// histórico de rastreamento para o objeto.
	if len(o.trackInfo) == 0 {
		return TrackInfo{}
	}

	return o.trackInfo[len(o.trackInfo)-1]
}

func (o *Object) GetTrackingHistory() []TrackInfo {
	return o.trackInfo
}
