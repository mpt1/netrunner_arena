#pragma once

#include <stdexcept>

enum class Faction
{
	Anarch = 0,
	Criminal,
	Shaper,
	HB,
	Jinteki,
	NBN,
	Weyland,
	Neutral,
	Adam,
	Sunny,
	Apex
};

enum class Pack
{
	Core = 0,
	WhatLiesAhead,
	TraceAmount,
	CyberExodus,
	AStudyInStatic,
	HumanitysShadow,
	FutureProof,
	CreationAndControl,
	OpeningMoves,
	SecondThoughts,
	MalaTempora,
	TrueColors,
	FearAndLoathing,
	DoubleTime,
	HonorAndProfit,
	Upstalk,
	TheSpacesBetween,
	FirstContact,
	UpAndOver,
	AllThatRemains,
	TheSource,
	OrderAndChaos,
	TheValley,
	BreakerBay,
	ChromeCity,
	TheUnderway,
	OldHollywood,
	TheUniverseOfTomorrow,
	DataAndDestiny,
	KalaGhoda,
	BusinessFirst,
	DemocracyAndDogma,
	SalsetteIsland,
	TheLiberatedMind,
	FearTheMasses,
	Seconds
};

std::string toString(Faction f)
{
	switch (f)
	{
		case Faction::Anarch: return "Anarch";
		case Faction::Criminal: return "Criminal";
		case Faction::Shaper: return "Shaper";
		case Faction::HB: return "HB";
		case Faction::Jinteki: return "Jinteki";
		case Faction::NBN: return "NBN";
		case Faction::Weyland: return "Weyland";
		case Faction::Neutral: return "Neutral";
		case Faction::Adam: return "Adam";
		case Faction::Sunny: return "Sunny";
		case Faction::Apex: return "Apex";
		default: return "Unknown";
	}
}

std::string toString(Pack p)
{
	switch (p)
	{
		case Pack::Core: return "Core";
		case Pack::WhatLiesAhead: return "WhatLiesAhead";
		case Pack::TraceAmount: return "TraceAmount";
		case Pack::CyberExodus: return "CyberExodus";
		case Pack::AStudyInStatic: return "AStudyInStatic";
		case Pack::HumanitysShadow: return "HumanitysShadow";
		case Pack::FutureProof: return "FutureProof";
		case Pack::CreationAndControl: return "CreationAndControl";
		case Pack::OpeningMoves: return "OpeningMoves";
		case Pack::SecondThoughts: return "SecondThoughts";
		case Pack::MalaTempora: return "MalaTempora";
		case Pack::TrueColors: return "TrueColors";
		case Pack::FearAndLoathing: return "FearAndLoathing";
		case Pack::DoubleTime: return "DoubleTime";
		case Pack::HonorAndProfit: return "HonorAndProfit";
		case Pack::Upstalk: return "Upstalk";
		case Pack::TheSpacesBetween: return "TheSpacesBetween";
		case Pack::FirstContact: return "FirstContact";
		case Pack::UpAndOver: return "UpAndOver";
		case Pack::AllThatRemains: return "AllThatRemains";
		case Pack::TheSource: return "TheSource";
		case Pack::OrderAndChaos: return "OrderAndChaos";
		case Pack::TheValley: return "TheValley";
		case Pack::BreakerBay: return "BreakerBay";
		case Pack::ChromeCity: return "ChromeCity";
		case Pack::TheUnderway: return "TheUnderway";
		case Pack::OldHollywood: return "OldHollywood";
		case Pack::TheUniverseOfTomorrow: return "TheUniverseOfTomorrow";
		case Pack::DataAndDestiny: return "DataAndDestiny";
		case Pack::KalaGhoda: return "KalaGhoda";
		case Pack::BusinessFirst: return "BusinessFirst";
		case Pack::DemocracyAndDogma: return "DemocracyAndDogma";
		case Pack::SalsetteIsland: return "SalsetteIsland";
		case Pack::TheLiberatedMind: return "TheLiberatedMind";
		case Pack::FearTheMasses: return "FearTheMasses";
		case Pack::Seconds: return "Seconds";
		default: return "Unknown";
	}
}

Pack PackFromString(std::string s)
{
	if (s == "Core") return Pack::Core;
	if (s == "WhatLiesAhead") return Pack::WhatLiesAhead;
	if (s == "TraceAmount") return Pack::TraceAmount;
	if (s == "CyberExodus") return Pack::CyberExodus;
	if (s == "AStudyInStatic") return Pack::AStudyInStatic;
	if (s == "HumanitysShadow") return Pack::HumanitysShadow;
	if (s == "FutureProof") return Pack::FutureProof;
	if (s == "CreationAndControl") return Pack::CreationAndControl;
	if (s == "OpeningMoves") return Pack::OpeningMoves;
	if (s == "SecondThoughts") return Pack::SecondThoughts;
	if (s == "MalaTempora") return Pack::MalaTempora;
	if (s == "TrueColors") return Pack::TrueColors;
	if (s == "FearAndLoathing") return Pack::FearAndLoathing;
	if (s == "DoubleTime") return Pack::DoubleTime;
	if (s == "HonorAndProfit") return Pack::HonorAndProfit;
	if (s == "Upstalk") return Pack::Upstalk;
	if (s == "TheSpacesBetween") return Pack::TheSpacesBetween;
	if (s == "FirstContact") return Pack::FirstContact;
	if (s == "UpAndOver") return Pack::UpAndOver;
	if (s == "AllThatRemains") return Pack::AllThatRemains;
	if (s == "TheSource") return Pack::TheSource;
	if (s == "OrderAndChaos") return Pack::OrderAndChaos;
	if (s == "TheValley") return Pack::TheValley;
	if (s == "BreakerBay") return Pack::BreakerBay;
	if (s == "ChromeCity") return Pack::ChromeCity;
	if (s == "TheUnderway") return Pack::TheUnderway;
	if (s == "OldHollywood") return Pack::OldHollywood;
	if (s == "TheUniverseOfTomorrow") return Pack::TheUniverseOfTomorrow;
	if (s == "DataAndDestiny") return Pack::DataAndDestiny;
	if (s == "KalaGhoda") return Pack::KalaGhoda;
	if (s == "BusinessFirst") return Pack::BusinessFirst;
	if (s == "DemocracyAndDogma") return Pack::DemocracyAndDogma;
	if (s == "SalsetteIsland") return Pack::SalsetteIsland;
	if (s == "TheLiberatedMind") return Pack::TheLiberatedMind;
	if (s == "FearTheMasses") return Pack::FearTheMasses;
	if (s == "Seconds") return Pack::Seconds;
	throw std::invalid_argument("Unkown Pack");
}

Faction FactionFromNRDBString(std::string in)
{
	if (in == "anarch") return Faction::Anarch;
	if (in == "criminal") return Faction::Criminal;
	if (in == "shaper") return Faction::Shaper;
	if (in == "haas-bioroid") return Faction::HB;
	if (in == "jinteki") return Faction::Jinteki;
	if (in == "nbn") return Faction::NBN;
	if (in == "weyland-consortium") return Faction::Weyland;
	if (in == "neutral") return Faction::Neutral;
	if (in == "neutral-runner") return Faction::Neutral;
	if (in == "neutral-corp") return Faction::Neutral;
	if (in == "adam") return Faction::Adam;
	if (in == "sunny-lebeau") return Faction::Sunny;
	if (in == "apex") return Faction::Apex;
	throw std::invalid_argument("Unkown Faction");
}

Pack PackFromNRDBString(std::string in)
{
	if (in == "core") return Pack::Core;
	if (in == "wla") return Pack::WhatLiesAhead;
	if (in == "ta") return Pack::TraceAmount;
	if (in == "ce") return Pack::CyberExodus;
	if (in == "asis") return Pack::AStudyInStatic;
	if (in == "hs") return Pack::HumanitysShadow;
	if (in == "fp") return Pack::FutureProof;
	if (in == "cac") return Pack::CreationAndControl;
	if (in == "om") return Pack::OpeningMoves;
	if (in == "st") return Pack::SecondThoughts;
	if (in == "mt") return Pack::MalaTempora;
	if (in == "tc") return Pack::TrueColors;
	if (in == "fal") return Pack::FearAndLoathing;
	if (in == "dt") return Pack::DoubleTime;
	if (in == "hap") return Pack::HonorAndProfit;
	if (in == "up") return Pack::Upstalk;
	if (in == "tsb") return Pack::TheSpacesBetween;
	if (in == "fc") return Pack::FirstContact;
	if (in == "uao") return Pack::UpAndOver;
	if (in == "atr") return Pack::AllThatRemains;
	if (in == "ts") return Pack::TheSource;
	if (in == "oac") return Pack::OrderAndChaos;
	if (in == "val") return Pack::TheValley;
	if (in == "bb") return Pack::BreakerBay;
	if (in == "cc") return Pack::ChromeCity;
	if (in == "uw") return Pack::TheUnderway;
	if (in == "oh") return Pack::OldHollywood;
	if (in == "uot") return Pack::TheUniverseOfTomorrow;
	if (in == "dad") return Pack::DataAndDestiny;
	if (in == "kg") return Pack::KalaGhoda;
	if (in == "bf") return Pack::BusinessFirst;
	if (in == "dag") return Pack::DemocracyAndDogma;
	if (in == "si") return Pack::SalsetteIsland;
	if (in == "tlm") return Pack::TheLiberatedMind;
	if (in == "ftm") return Pack::FearTheMasses;
	if (in == "23s") return Pack::Seconds;
	throw std::invalid_argument("Unkown Pack");
}

