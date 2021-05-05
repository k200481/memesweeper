#pragma once

#include "Graphics.h"
#include "Vei2.h"
#include "RectI.h"
#include "SoundEffect.h"

class MemeField
{
private: //private utility class
	enum class Fate {
		GonnaLive,
		GonnaDie,
		YetUncertain
	};
	class Tile {
	public: //public utility enum
		enum class State {
			Hidden,
			Revealed,
			Flagged
		};

	public: //public functions
		bool IsRevealed() const;
		bool IsHidden() const;
		bool IsFlagged() const;
		bool HasMeme() const;
		bool HasNoNeighbors() const;
		void Draw( Vei2 screenPos, Fate fate, Graphics& gfx ) const;

		void reveal();
		void toggleFlag();
		void spawnMeme();

		void setNumAdjacentMemes( int num );

	private:
		State state = State::Hidden;
		bool hasMeme = false;
		int numAdjacentMemes = -1;
	};

public: //public functions
	MemeField( Vei2 center, int nMemes );
	RectI getRect() const;

	void OnRevealClick( const Vei2& screenPos );
	void OnFlagClick( const Vei2& screenPos );

	bool IsOnField( const Vei2& screenPos ) const;

	void Draw( Graphics& gfx ) const;

private: //utility functions
	int countAdjacentMemes( const Vei2& gridPos ) const;

	void RevealTiles( const Vei2& gridPos );
	bool GameIsWon() const;

	Vei2 GridToScreen(const Vei2& gridPos) const;
	Vei2 ScreenToGrid(const Vei2& screenPos) const;
	int GridToIndex(const Vei2& gridPos) const;
	Vei2 IndexToGrid( const int i ) const;

private: //private variables
	static constexpr unsigned int width = 10u;
	static constexpr unsigned int height = 10u;
	RectI board;
	Tile tiles[width * height];

	Fate playerFate = Fate::YetUncertain;
};

