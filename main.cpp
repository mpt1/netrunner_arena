/* Android: Netrunner - Arena drafting */
constexpr int BUILD_NUMBER = 446;

/* Dependency: JSON for Modern C++
 * https://github.com/nlohmann/json
 */
#include <json.hpp>
using json = nlohmann::json;

/* Dependency: Bloat-free Immediate Mode Graphical User interface for C++ 
 * https://github.com/ocornut/imgui
 */
#include <imgui.h>
#include "imgui_impl_glfw.h"

 /* Dependency: GLFW is a free, Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan application development.
 * http://www.glfw.org/
 */
#include <GLFW/glfw3.h>

 /* Dependency: stb single-file public domain libraries for C/C++
 * https://github.com/nothings/stb
 */
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb_image.h>

#include <random>
#include <chrono>
#include <iostream>
#include <fstream>
#include <future>
#include <algorithm>
#include <cassert>

#include "enums.h"

std::default_random_engine g_rng;

constexpr size_t g_max_cards = 5120;
GLuint g_texture[g_max_cards];
GLuint g_side_tex[2];

class Card
{
public:
	Faction faction;
	int influence;
	int agenda_points;
	std::string name;
	std::string type;
	int pack_number;
	Pack pack;
	int copies;
	GLuint texId;
};

class Deck
{
public:
	Card identity;
	std::vector<Card> cards;
};

enum class GuiState
{
	Start,
	SideSelect,
	CorpFactionSelect,
	RunnerFactionSelect,
	CorpCardsSelect,
	RunnerCardsSelect,
	Summary
};

// Available cards and ids, unavailable cards will be removed during drafting
std::vector<Card> g_corp_ids;
std::vector<Card> g_runner_ids;
std::vector<Card> g_corp_cards;
std::vector<Card> g_runner_cards;

// card statistics
std::map<int, std::pair<int, int>> g_stats;

// user options:
// allow duplicates in selection?
bool opt_allowDuplicates = false;
// meaning of influence
enum OPT_INFLUENCE
{
	OPT_NORMAL = 0,  // normal influence rules
	OPT_NOINF,		 // influence has no meaning
	OPT_DISALLOW3INF // ensure atleast one 0 influence card in choice
};
int opt_influence = OPT_NORMAL;
// draw more than minimum deck size?
int opt_plusCards = 0;
// selected packs
std::map<Pack, int> g_allowed_packs;

// draw and remove a random card from the deck vector
// reqs contains optional requierments 
Card draw_card(std::vector<Card>& deck, std::vector<std::function<bool(Card)>> reqs = std::vector<std::function<bool(Card)>>())
{
	// TODO: gracefully handling
	assert(deck.size() > 0 && "Can't draw from empty deck.");
	size_t pos;

	std::vector<size_t> req_deck;
	for (size_t i = 0; i < deck.size(); i++)
	{
		bool skip = false;
		for (auto req : reqs) if (!req(deck[i])) { skip = true; break; }
		if (!skip) req_deck.push_back(i);
	}
	assert(req_deck.size() > 0 && "Can't draw from filtered deck.");
	std::uniform_int_distribution<size_t> distribution(0, req_deck.size() - 1);
	pos = distribution(g_rng);
	pos = req_deck.at(pos);

	Card c = deck.at(pos);
	deck.erase(deck.begin() + pos);
	return c;
}

int influenceCost(Card c, Faction id)
{
	if (c.faction == id) return 0;
	return c.influence;
}

// get min deck size, available influence and corp points for a specific identity.
// remove Jinteki cards when using "Custom Biotics"
// TODO: read this information from nrdb json dump 
void setIdentity(Card identity, int& cards, int& influence, int& points)
{
	cards = 45;
	influence = 15;
	if (identity.pack == Pack::CreationAndControl && identity.pack_number == 2)						/* identity.name == "Custom Biotics: Engineered for Success" */
	{
		influence = 22;
		g_corp_cards.erase(std::remove_if(g_corp_cards.begin(), g_corp_cards.end(), [](const Card& c) { return c.faction == Faction::Jinteki; }), g_corp_cards.end());
	}
	if (identity.pack == Pack::ChromeCity && identity.pack_number == 50) cards = 40;				/* identity.name == "Cybernetics Division: Humanity Upgraded" */
	if (identity.pack == Pack::FearAndLoathing && identity.pack_number == 97) influence = 10;		/* identity.name == "GRNDL: Power Unleashed" */
	if (identity.pack == Pack::HonorAndProfit && identity.pack_number == 1)							/* identity.name == "Harmony Medtech: Biomedical Pioneer" */
	{
		cards = 40;
		influence = 12;
	}
	if (identity.pack == Pack::FutureProof && identity.pack_number == 114)							/* identity.name == "NBN: The World is Yours*" */
	{
		cards = 40;
		influence = 12;
	}
	if (identity.pack == Pack::Upstalk && identity.pack_number == 5) influence = 17;				/* identity.name == "Near-Earth Hub: Broadcast Center" */
	if (identity.pack == Pack::CreationAndControl && identity.pack_number == 3) influence = 12;		/* identity.name == "NEXT Design: Guarding the Net" */
	if (identity.pack == Pack::DataAndDestiny && identity.pack_number == 1) cards = 40;				/* identity.name == "SYNC: Everything, Everywhere" */
	if (identity.pack == Pack::OrderAndChaos && identity.pack_number == 3) influence = 17;			/* identity.name == "Titan Transnational: Investing In Your Future" */
	if (identity.pack == Pack::Seconds && identity.pack_number == 17) influence = 12;				/* identity.name == "NBN: Controlling the Message" */

	if (identity.pack == Pack::CyberExodus && identity.pack_number == 46) cards = 40;				/* identity.name == "Chaos Theory: W\u00fcnderkind" */
	if (identity.pack == Pack::HonorAndProfit && identity.pack_number == 28) influence = 10;		/* identity.name == "Iain Stirling: Retired Spook" */
	if (identity.pack == Pack::HonorAndProfit && identity.pack_number == 29) influence = 17;		/* identity.name == "Ken \"Express\" Tenma: Disappeared Clone" */
	if (identity.pack == Pack::CreationAndControl && identity.pack_number == 28) influence = 10;	/* identity.name == "Rielle \"Kit\" Peddler: Transhuman" */
	if (identity.pack == Pack::HonorAndProfit && identity.pack_number == 30) cards = 40;			/* identity.name == "Silhouette: Stealth Operative" */
	if (identity.pack == Pack::CreationAndControl && identity.pack_number == 29) influence = 1;		/* identity.name == "The Professor: Keeper of Knowledge" */
	if (identity.pack == Pack::OrderAndChaos && identity.pack_number == 30) cards = 50;				/* identity.name == "Valencia Estevez: The Angel of Cayambe" */
	if (identity.pack == Pack::DataAndDestiny && identity.pack_number == 29) influence = 25;		/* identity.name == "Apex: Invasive Predator" */
	if (identity.pack == Pack::DataAndDestiny && identity.pack_number == 37) influence = 25;		/* identity.name == "Adam: Compulsive Hacker" */
	if (identity.pack == Pack::DataAndDestiny && identity.pack_number == 45)						/* identity.name == "Sunny Lebeau: Security Specialist" */
	{
		cards = 50;
		influence = 25;
	}

	cards += opt_plusCards;

	points = 19 + 2 * ((cards - 40) / 5);
}

// read json file (nrdb dump) from file "data"
// loads images from "img" directory
bool read_data()
{
	glGenTextures(2, &g_side_tex[0]);
	std::string side_fn[2] = { "img/corp.png", "img/runner.png" };
	for (size_t i = 0; i < 2; i++)
	{
		int sizeX, sizeY, bpp;
		unsigned char* data = stbi_load(side_fn[i].c_str(), &sizeX, &sizeY, &bpp, 4);
		if (data == 0)
		{
			std::cerr << "Could not find the picture for " << side_fn[i] << "\n";
			continue;
		}
		glBindTexture(GL_TEXTURE_2D, g_side_tex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glGenTextures(g_max_cards, &g_texture[0]);
	std::ifstream fin("data");
	if (!fin.good()) return false;
	std::string str;
	fin.seekg(0, std::ios::end);
	str.reserve(static_cast<size_t>(fin.tellg()));
	fin.seekg(0, std::ios::beg);
	str.assign((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
	fin.close();

	json j;
	auto future_j = std::async(std::launch::async, [&str]() { return json::parse(str); });
	char spinner[] = "/-\\|";
	size_t s = 0;
	std::cout << "Loading data ... ";
	while (future_j.wait_for(std::chrono::milliseconds(100)) == std::future_status::timeout)
	{
		std::cout << "\b" << spinner[s];
		s = (s + 1) % 4;
	}
	try
	{
		j = future_j.get();
		std::cout << "\bdone.\n";
	}
	catch (std::invalid_argument& e) { std::cerr << "Corrupted data file. " << e.what() << "\n"; return false; }
	size_t r = 0;

	std::cout << "Loading images ... ";
	for (auto x : j)
	{
		Card c;
		try
		{
			if (x.count("agendapoints") > 0) c.agenda_points = x["agendapoints"].get<int>();
			else c.agenda_points = -1;
			c.copies = x["quantity"].get<int>();
			c.faction = FactionFromNRDBString(x["faction_code"].get<std::string>());
			if (x.count("factioncost") > 0) c.influence = x["factioncost"].get<int>();
			else c.influence = -1;
			c.name = x["title"].get<std::string>();
			c.pack = PackFromNRDBString(x["set_code"].get<std::string>());
			c.pack_number = x["number"].get<int>();

			int copies = c.copies;
			if (g_allowed_packs.count(c.pack) > 0) copies = std::min(3, copies * g_allowed_packs[c.pack]);
			else continue;

			c.type = x["type_code"].get<std::string>();

			std::string filename = x["imagesrc"].get<std::string>();
			size_t delimit = filename.find_last_of('/');
			if (delimit != std::string::npos) filename = filename.substr(delimit + 1);
			filename = "img/" + filename;
			int sizeX, sizeY, bpp;
			unsigned char* data = stbi_load(filename.c_str(), &sizeX, &sizeY, &bpp, 4);
			c.texId = 0;
			if (data == 0) std::cerr << "Could not find the picture for " << c.name << "\n";
			else
			{
				assert(r < g_max_cards);
				glBindTexture(GL_TEXTURE_2D, g_texture[r]);
				glTexImage2D(GL_TEXTURE_2D, 0, 4, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				c.texId = g_texture[r];
				stbi_image_free(data);
				r++;
				std::cout << "\b" << spinner[s];
				s = (s + 1) % 4;
			}

			if (x["side_code"] == "runner")
			{
				if (x["type_code"] == "identity" && copies > 0) g_runner_ids.push_back(c);
				else for (int i = 0; i<copies; i++) g_runner_cards.push_back(c);
			}
			else if (x["side_code"] == "corp")
			{
				if (x["type_code"] == "identity" && copies > 0) g_corp_ids.push_back(c);
				else for (int i = 0; i<copies; i++) g_corp_cards.push_back(c);
			}
		}
		catch (std::invalid_argument& e)
		{
			std::cerr << "Unkown card: " << x.dump(2) << "\n" << e.what() << "\n";
		}
	}
	std::cout << "\bdone.\n";
	return true;
}

// read stats from file "stats.txt". format: card id, #picked, #offered
bool read_stats()
{
	std::ifstream fin("stats.txt");
	if (!fin.good()) return false;
	std::string in;
	while (getline(fin, in))
	{
		in = in.substr(0, in.find_first_of('#'));
		std::stringstream ss(in);
		int cardid, picked, offered;
		ss >> cardid >> picked >> offered;
		g_stats[cardid] = std::make_pair(picked, offered);
	}
	fin.close();
	return true;
}

bool write_stats()
{
	std::ofstream fout("stats.txt");
	if (!fout.good()) return false;
	for (auto& x : g_stats)
	{
		fout << x.first << " " << x.second.first << " " << x.second.second << "\n";
	}
	fout.close();
	return true;
}


// read options and selected packs from file "packs.txt", treat '#' as single line comments
bool read_packs()
{
	std::ifstream fin("packs.txt");
	if (!fin.good()) return false;
	std::string in;
	while (getline(fin, in))
	{
		in = in.substr(0, in.find_first_of('#'));
		std::stringstream ss(in);
		std::string s;
		int v = 0;
		ss >> s >> v;
		if (s == "OPT_INFLUENCE") opt_influence = v;
		else if (s == "OPT_ALLOWDUPLICATES") opt_allowDuplicates = static_cast<bool>(v);
		else if (s == "OPT_PLUSCARDS") opt_plusCards = v;
		else if (v != 0)
		{
			try { g_allowed_packs[PackFromString(s)] = v; }
			catch (std::invalid_argument& e) { std::cerr << "Unkown pack in packs.txt: " << s << "\n" << e.what() << "\n"; }
		}
	}
	fin.close();
	return true;
}

bool write_packs()
{
	std::ofstream fout("packs.txt");
	if (!fout.good()) return false;
	fout << "OPT_INFLUENCE " << opt_influence << "\nOPT_ALLOWDUPLICATES " << opt_allowDuplicates << "\nOPT_PLUSCARDS " << opt_plusCards << "\n";
	for (auto& x : g_allowed_packs)
	{
		fout << toString(x.first) << " " << x.second << "\n";
	}
	fout.close();
	return true;
}

// callback for GLFW errors
static void error_callback(int error, const char* description)
{
	std::cerr << "Error " << error << ": " << description << "\n";
}


int main()
{
	g_rng.seed(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

	// Setup window
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) return 1;
	GLFWwindow* window = glfwCreateWindow(1351, 644, "Netrunner Arena", NULL, NULL);
	glfwMakeContextCurrent(window);
	// Setup ImGui binding
	ImGui_ImplGlfw_Init(window, true);
	// Load Fonts
	ImGuiIO& io = ImGui::GetIO();
	static const ImWchar ranges[] =
	{
		0x0001, 0x001F,
		0x0020, 0x077F,
		0x0780, 0x139F,
		0x13A0, 0x1DBF,
		0x1DC0, 0x257F,
		0x2580, 0x2DFF,
		0x2E00, 0x4DBF,
		//0x4DC0, 0xFAFF,
		0xFB00, 0xFFFF,
		0,
	};
	// Dependency: Unicode font
	// http://unifoundry.com/unifont.html
	io.Fonts->AddFontFromFileTTF("unifont.ttf", 18.0f, NULL, &ranges[0]);

	if (!read_stats()) std::cerr << "No stats.txt found. Creating new statistics file.\n";
	if (!read_packs()) std::cerr << "No packs.txt found. Creating new options file.\n";

	Deck deck;
	Card choices[3];
	std::string str_choices[3];
	bool drawCards = true;
	int cards = 45;
	int influence = 15;
	int points = 21;

	// Main loop
	std::string statusLine;
	GuiState guiState = GuiState::Start;
	ImVec4 clear_color = ImColor(114, 144, 154);
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		ImGui_ImplGlfw_NewFrame();

		ImGui::SetNextWindowSize(ImVec2(993, 644), ImGuiSetCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(0, 0), 0);
		ImGui::SetNextWindowCollapsed(false, 0);
		ImGui::Begin("Netrunner Arena");

		if (guiState == GuiState::Start)
		{
			ImGui::Text(R"(
   _  __      __                                 
  / |/ /___  / /_ ____ __ __ ___   ___  ___  ____
 /    // -_)/ __// __// // // _ \ / _ \/ -_)/ __/
/_/|_/ \__/ \__//_/   \_,_//_//_//_//_/\__//_/   
   ___    ___   ____ _  __ ___                   
  / _ |  / _ \ / __// |/ // _ |                  
 / __ | / , _// _/ /    // __ |                  
/_/ |_|/_/|_|/___//_/|_//_/ |_|                  
                                                 
Build a deck by repeatedly choosing 1 out of 3 cards.

)");
			ImGui::Text("Build %d", BUILD_NUMBER);

			if (ImGui::TreeNode("Packs"))
			{
				static int numCore = std::max(1, g_allowed_packs[Pack::Core]);
				ImGui::RadioButton("Core", &numCore, 1); ImGui::SameLine();
				ImGui::RadioButton("x2", &numCore, 2); ImGui::SameLine();
				ImGui::RadioButton("x3", &numCore, 3);
				if (ImGui::TreeNode("Genesis"))
				{
					ImGui::Selectable("What Lies Ahead", (bool*)(&g_allowed_packs[Pack::WhatLiesAhead]));
					ImGui::Selectable("Trace Amount", (bool*)(&g_allowed_packs[Pack::TraceAmount]));
					ImGui::Selectable("Cyber Exodus", (bool*)(&g_allowed_packs[Pack::CyberExodus]));
					ImGui::Selectable("A Study in Static", (bool*)(&g_allowed_packs[Pack::AStudyInStatic]));
					ImGui::Selectable("Humanity's Shadow", (bool*)(&g_allowed_packs[Pack::HumanitysShadow]));
					ImGui::Selectable("Future Proof", (bool*)(&g_allowed_packs[Pack::FutureProof]));
					ImGui::TreePop();
				}
				ImGui::Selectable("Creation and Control", (bool*)(&g_allowed_packs[Pack::CreationAndControl]));
				if (ImGui::TreeNode("Spin"))
				{
					ImGui::Selectable("Opening Moves", (bool*)(&g_allowed_packs[Pack::OpeningMoves]));
					ImGui::Selectable("Second Thoughts", (bool*)(&g_allowed_packs[Pack::SecondThoughts]));
					ImGui::Selectable("Mala Tempora", (bool*)(&g_allowed_packs[Pack::MalaTempora]));
					ImGui::Selectable("True Colors", (bool*)(&g_allowed_packs[Pack::TrueColors]));
					ImGui::Selectable("Fear and Loathing", (bool*)(&g_allowed_packs[Pack::FearAndLoathing]));
					ImGui::Selectable("Double Time", (bool*)(&g_allowed_packs[Pack::DoubleTime]));
					ImGui::TreePop();
				}
				ImGui::Selectable("Honor and Profit", (bool*)(&g_allowed_packs[Pack::HonorAndProfit]));
				if (ImGui::TreeNode("Lunar"))
				{
					ImGui::Selectable("Upstalk", (bool*)(&g_allowed_packs[Pack::Upstalk]));
					ImGui::Selectable("The Spaces Between", (bool*)(&g_allowed_packs[Pack::TheSpacesBetween]));
					ImGui::Selectable("First Contact", (bool*)(&g_allowed_packs[Pack::FirstContact]));
					ImGui::Selectable("Up and Over", (bool*)(&g_allowed_packs[Pack::UpAndOver]));
					ImGui::Selectable("All That Remains", (bool*)(&g_allowed_packs[Pack::AllThatRemains]));
					ImGui::Selectable("The Source", (bool*)(&g_allowed_packs[Pack::TheSource]));
					ImGui::TreePop();
				}
				ImGui::Selectable("Order and Chaos", (bool*)(&g_allowed_packs[Pack::OrderAndChaos]));
				if (ImGui::TreeNode("SanSan"))
				{
					ImGui::Selectable("The Valley", (bool*)(&g_allowed_packs[Pack::TheValley]));
					ImGui::Selectable("Breaker Bay", (bool*)(&g_allowed_packs[Pack::BreakerBay]));
					ImGui::Selectable("Chrome City", (bool*)(&g_allowed_packs[Pack::ChromeCity]));
					ImGui::Selectable("The Underway", (bool*)(&g_allowed_packs[Pack::TheUnderway]));
					ImGui::Selectable("Old Hollywood", (bool*)(&g_allowed_packs[Pack::OldHollywood]));
					ImGui::Selectable("The Universe of Tomorrow", (bool*)(&g_allowed_packs[Pack::TheUniverseOfTomorrow]));
					ImGui::TreePop();
				}
				ImGui::Selectable("Data and Destiny", (bool*)(&g_allowed_packs[Pack::DataAndDestiny]));
				if (ImGui::TreeNode("Mumbad"))
				{
					ImGui::Selectable("Kala Ghoda", (bool*)(&g_allowed_packs[Pack::KalaGhoda]));
					ImGui::Selectable("Business First", (bool*)(&g_allowed_packs[Pack::BusinessFirst]));
					ImGui::Selectable("Democracy and Dogma", (bool*)(&g_allowed_packs[Pack::DemocracyAndDogma]));
					ImGui::Selectable("Salsette Island", (bool*)(&g_allowed_packs[Pack::SalsetteIsland]));
					ImGui::Selectable("The Liberated Mind", (bool*)(&g_allowed_packs[Pack::TheLiberatedMind]));
					ImGui::Selectable("Fear the Masses", (bool*)(&g_allowed_packs[Pack::FearTheMasses]));
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Flashpoint"))
				{
					ImGui::Selectable("23 Seconds", (bool*)(&g_allowed_packs[Pack::Seconds]));
					ImGui::TreePop();
				}

				g_allowed_packs[Pack::Core] = numCore;
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Options"))
			{
				ImGui::Selectable("Allow duplicates in selection", &opt_allowDuplicates);
				ImGui::RadioButton("Normal", &opt_influence, OPT_NORMAL); ImGui::SameLine();
				ImGui::RadioButton("At least one zero influence card in selection", &opt_influence, OPT_DISALLOW3INF); ImGui::SameLine();
				ImGui::RadioButton("Influence has no meaning", &opt_influence, OPT_NOINF);
				ImGui::InputInt("Draft more than minimum deck size.", &opt_plusCards, 1, 5);
				opt_plusCards = std::max(std::min(opt_plusCards, 20), 0);
				ImGui::TreePop();
			}

			if (ImGui::Button("Start"))
			{
				guiState = GuiState::SideSelect;
				if (!read_data())
				{
					std::cerr << "No data file found, aborting.\n";
					return 1;
				}
			}
			ImGui::SameLine(200.f);
			if (ImGui::Button("Quit")) { break; }
		}

		else if (guiState == GuiState::SideSelect)
		{
			ImGui::Columns(2, 0, false);
			for (size_t i = 0; i < 2; i++)
			{
				ImGui::PushID(i);
				if (ImGui::ImageButton((void*)g_side_tex[i], ImVec2(300, 418), ImVec2(0, 0), ImVec2(1, 1), 0))
				{
					if (i == 0) guiState = GuiState::CorpFactionSelect;
					else if (i == 1) guiState = GuiState::RunnerFactionSelect;
				}
				ImGui::PopID();
				ImGui::NextColumn();
			}
			ImGui::Text("Corporation");
			ImGui::NextColumn();
			ImGui::Text("Runner");
			ImGui::NextColumn();
			ImGui::Columns(1);
			if (ImGui::Button("Random"))
			{
				std::uniform_int_distribution<size_t> distribution(0, 1);
				if (distribution(g_rng) == 0)
				{
					guiState = GuiState::CorpFactionSelect;
					statusLine = "I picked 'Corporation' for you!";
				}
				else
				{
					guiState = GuiState::RunnerFactionSelect;
					statusLine = "I picked 'Runner' for you!";
				}
			}
		}

		// id
		else if (guiState == GuiState::CorpFactionSelect || guiState == GuiState::RunnerFactionSelect)
		{
			if (drawCards)
			{
				drawCards = false;
				for (size_t i = 0; i < 3; i++)
				{
					choices[i] = draw_card((guiState == GuiState::CorpFactionSelect) ? g_corp_ids : g_runner_ids);
					str_choices[i] = choices[i].name;
					g_stats[static_cast<int>(choices[i].pack) * 1000 + choices[i].pack_number].second++;
				}
			}
			ImGui::Columns(3, 0, false);
			for (size_t i = 0; i < 3; i++)
			{
				ImGui::PushID(i);
				if (ImGui::ImageButton((void*)choices[i].texId, ImVec2(300, 418)))
				{
					statusLine = "";
					guiState = (guiState == GuiState::CorpFactionSelect) ? GuiState::CorpCardsSelect : GuiState::RunnerCardsSelect;
					drawCards = true;
					deck.identity = choices[i];
					setIdentity(deck.identity, cards, influence, points);
					g_stats[static_cast<int>(deck.identity.pack) * 1000 + deck.identity.pack_number].first++;
				}
				ImGui::PopID();
				ImGui::NextColumn();
			}
			for (size_t i = 0; i < 3; i++)
			{
				ImGui::TextWrapped(str_choices[i].c_str());
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
			ImGui::PushID(3);
			if (ImGui::Button("Random"))
			{
				guiState = (guiState == GuiState::CorpFactionSelect) ? GuiState::CorpCardsSelect : GuiState::RunnerCardsSelect;
				drawCards = true;
				std::uniform_int_distribution<size_t> distribution(0, 2);
				deck.identity = choices[distribution(g_rng)];
				statusLine = "I picked '" + deck.identity.name + "' for you!";
				setIdentity(deck.identity, cards, influence, points);
				g_stats[static_cast<int>(deck.identity.pack) * 1000 + deck.identity.pack_number].first++;
			}
			ImGui::PopID();
		}

		// cards
		else if (guiState == GuiState::CorpCardsSelect || guiState == GuiState::RunnerCardsSelect)
		{
			if (drawCards)
			{
				drawCards = false;
				g_corp_cards.erase(std::remove_if(g_corp_cards.begin(), g_corp_cards.end(),
					[deck](const Card& c) { return (influenceCost(c, deck.identity.faction) == -1); }),
					g_corp_cards.end());
				g_runner_cards.erase(std::remove_if(g_runner_cards.begin(), g_runner_cards.end(),
					[deck](const Card& c) { return (influenceCost(c, deck.identity.faction) == -1); }),
					g_runner_cards.end());
				g_corp_cards.erase(std::remove_if(g_corp_cards.begin(), g_corp_cards.end(),
					[points](const Card& c) { return (c.agenda_points != -1 && c.agenda_points > points); }),
					g_corp_cards.end());
				if (opt_influence != OPT_NOINF) g_corp_cards.erase(std::remove_if(g_corp_cards.begin(), g_corp_cards.end(),
					[deck, influence](const Card& c) { return (deck.identity.faction != c.faction && (c.influence > influence)); }),
					g_corp_cards.end());
				if (opt_influence != OPT_NOINF) g_runner_cards.erase(std::remove_if(g_runner_cards.begin(), g_runner_cards.end(),
					[deck, influence](const Card& c) { return (deck.identity.faction != c.faction && (c.influence > influence)); }),
					g_runner_cards.end());

				for (size_t i = 0; i < 3; i++)
				{
					std::vector<std::function<bool(Card)>> reqs;
					if (!opt_allowDuplicates) reqs.push_back([&i, &choices](Card c) { for (size_t j = 0; j < i; j++) if (c.name == choices[j].name) return false; return true; });
					if (i == 2 && opt_influence == OPT_DISALLOW3INF) reqs.push_back(
						[&deck, &choices](Card c) { if (influenceCost(choices[0], deck.identity.faction) > 0 && influenceCost(choices[1], deck.identity.faction) > 0
							&& influenceCost(c, deck.identity.faction) > 0) return false; return true; });
					if (guiState == GuiState::CorpCardsSelect && points - 1 >= cards)
					{
						int need = (points - 1 + cards - 1) / cards;
						reqs.push_back([&need](Card c) { return c.agenda_points >= need; });
					}
					choices[i] = draw_card((guiState == GuiState::CorpCardsSelect) ? g_corp_cards : g_runner_cards, reqs);
					str_choices[i] = choices[i].name;
					g_stats[static_cast<int>(choices[i].pack) * 1000 + choices[i].pack_number].second++;
				}
			}
			ImGui::Columns(3, 0, false);
			for (size_t i = 0; i < 3; i++)
			{
				ImGui::PushID(i);
				if (ImGui::ImageButton((void*)choices[i].texId, ImVec2(300, 418)))
				{
					statusLine = "";
					drawCards = true;
					if (choices[i].faction != deck.identity.faction && choices[i].influence != -1) influence -= choices[i].influence;
					if (choices[i].agenda_points != -1) points -= choices[i].agenda_points;
					for (size_t j = 0; j < 3; j++) if (j != i) ((guiState == GuiState::CorpCardsSelect) ? g_corp_cards : g_runner_cards).push_back(choices[j]);
					deck.cards.push_back(choices[i]);
					// Erreta 3.1, limit AstroScript to one per deck.
					if (choices[i].pack == Pack::Core && choices[i].pack_number == 81) g_corp_cards.erase(std::remove_if(g_corp_cards.begin(), g_corp_cards.end(),
						[deck](const Card& c) { return c.pack == Pack::Core && c.pack_number == 81; }), g_corp_cards.end());
					cards--;
					g_stats[static_cast<int>(choices[i].pack) * 1000 + choices[i].pack_number].first++;
					if (cards <= 0) guiState = GuiState::Summary;
				}
				ImGui::PopID();
				ImGui::NextColumn();
			}
			for (size_t i = 0; i < 3; i++)
			{
				ImGui::TextWrapped(str_choices[i].c_str());
				ImGui::NextColumn();
			}
			for (size_t i = 0; i < 3; i++)
			{
				std::string s = "";
				if (choices[i].faction != deck.identity.faction)
				{
					s = toString(choices[i].faction);
					if (choices[i].faction != Faction::Neutral) s += ", Influence: " + std::to_string(choices[i].influence);
				}
				ImGui::Text(s.c_str());
				ImGui::NextColumn();
			}
			for (size_t i = 0; i < 3; i++)
			{
				std::string s = "";
				if (choices[i].agenda_points >= 0) s = "Agenda points: " + std::to_string(choices[i].agenda_points);
				ImGui::Text(s.c_str());
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
			if (guiState == GuiState::CorpCardsSelect) ImGui::Text("\n Cards left: %d     Influence left: %d     Agenda points left: %d", cards, influence, points);
			else ImGui::Text("\n Cards left: %d     Influence left: %d", cards, influence);
		}

		// end
		else if (guiState == GuiState::Summary)
		{
			ImGui::Text(deck.identity.name.c_str());
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Image((void*)deck.identity.texId, ImVec2(300, 418));
				ImGui::EndTooltip();
			}
			std::sort(deck.cards.begin(), deck.cards.end(), [](Card& l, Card& r) { if (l.faction == r.faction) return l.name < r.name; return l.faction < r.faction; });
			size_t i = 0;
			while (i < deck.cards.size())
			{
				Faction fact = deck.cards[i].faction;
				int fcount = 0;
				size_t j = i;
				while (j < deck.cards.size() && fact == deck.cards[j].faction)
				{
					fcount++;
					j++;
				}
				if (ImGui::TreeNode(toString(deck.cards[i].faction).c_str(), "%s x%d", toString(deck.cards[i].faction).c_str(), fcount))
				{
					while (i < deck.cards.size() && fact == deck.cards[i].faction)
					{
						int count = 1;
						while (i + 1 < deck.cards.size() && deck.cards[i].name == deck.cards[i + 1].name)
						{
							i++;
							count++;
						}
						if (count > 1) ImGui::Text("%s x%d", deck.cards[i].name.c_str(), count);
						else ImGui::Text(deck.cards[i].name.c_str());
						if (ImGui::IsItemHovered())
						{
							ImGui::BeginTooltip();
							ImGui::Image((void*)deck.cards[i].texId, ImVec2(300, 418));
							ImGui::EndTooltip();
						}
						i++;
					}
					ImGui::TreePop();
				}
				else while (i < deck.cards.size() && fact == deck.cards[i].faction) i++;
			}
			static char filename[128] = "deck.txt";
			if (ImGui::Button("Save"))
			{
				std::ofstream fout;
				fout.open(filename);
				if (!fout.good()) std::cerr << "\nCould not open file: " << filename << "\n";
				else
				{
					fout << "1 " << deck.identity.name << " (" << toString(deck.identity.pack) << ", " << deck.identity.pack_number << ")\n";
					for (auto& c : deck.cards) fout << "1 " << c.name << " (" << toString(c.pack) << ", " << c.pack_number << ")\n";
					fout.close();
				}
			}
			ImGui::SameLine();
			ImGui::InputText("", filename, 128);
			if (ImGui::Button("Quit")) break;
		}
		ImGui::Spacing();
		ImGui::Text(statusLine.c_str());
		ImGui::End();

		if (guiState == GuiState::CorpCardsSelect || guiState == GuiState::RunnerCardsSelect)
		{
			ImGui::SetNextWindowSize(ImVec2(358, 644), ImGuiSetCond_FirstUseEver);
			ImGui::SetNextWindowPos(ImVec2(993, 0), 0);
			ImGui::SetNextWindowCollapsed(false, 0);
			ImGui::Begin("Deck");
			ImGui::Text(deck.identity.name.c_str());
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Image((void*)deck.identity.texId, ImVec2(300, 418));
				ImGui::EndTooltip();
			}
			std::sort(deck.cards.begin(), deck.cards.end(), [](Card& l, Card& r) { if (l.type == r.type) return l.name < r.name; return l.type < r.type; });
			size_t i = 0;
			while (i < deck.cards.size())
			{
				std::string type = deck.cards[i].type;
				if (ImGui::TreeNode(type.c_str()))
				{
					while (i < deck.cards.size() && type == deck.cards[i].type)
					{
						int count = 1;
						while (i + 1 < deck.cards.size() && deck.cards[i].name == deck.cards[i + 1].name)
						{
							i++;
							count++;
						}
						if (count > 1) ImGui::Text("%s x%d", deck.cards[i].name.c_str(), count);
						else ImGui::Text(deck.cards[i].name.c_str());

						if (ImGui::IsItemHovered())
						{
							ImGui::BeginTooltip();
							ImGui::Image((void*)deck.cards[i].texId, ImVec2(300, 418));
							ImGui::EndTooltip();
						}
						i++;
					}
					ImGui::TreePop();
				}
				else while (i < deck.cards.size() && type == deck.cards[i].type) i++;
			}
			ImGui::End();
		}

		// Rendering
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui::Render();
		glfwSwapBuffers(window);
	}

	// Cleanup
	ImGui_ImplGlfw_Shutdown();
	glfwTerminate();

	write_packs();
	write_stats();

	return 0;
}