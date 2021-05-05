#include "MemeField.h"
#include <random>
#include <assert.h>
#include <algorithm>

#include "SpriteCodex.h"

bool MemeField::Tile::IsRevealed() const
{
	return state == State::Revealed;
}

bool MemeField::Tile::IsHidden() const
{
	return state == State::Hidden;
}

bool MemeField::Tile::IsFlagged() const
{
	return state == State::Flagged;
}

bool MemeField::Tile::HasMeme() const
{
	return hasMeme;
}

bool MemeField::Tile::HasNoNeighbors() const
{
	return numAdjacentMemes == 0;
}

void MemeField::Tile::Draw(Vei2 screenPos, Fate fate, Graphics& gfx) const
{
	assert( screenPos.x >= 0 && screenPos.x < Graphics::ScreenWidth && 
		screenPos.y >= 0 && screenPos.y < Graphics::ScreenHeight );
	assert( screenPos.x + SpriteCodex::tileSize < Graphics::ScreenWidth &&
		screenPos.y + SpriteCodex::tileSize < Graphics::ScreenHeight );
	
	switch ( state ) {
	case State::Hidden:
		
		if ( fate == Fate::GonnaDie && HasMeme() ) {
			SpriteCodex::DrawTileBomb( screenPos, gfx );
		}
		else {
			SpriteCodex::DrawTileButton( screenPos, gfx );
		}
		break;
	case State::Flagged:
		if ( fate == Fate::GonnaDie && !HasMeme() ) {
			SpriteCodex::DrawTileCross( screenPos, gfx );
		}
		else if ( fate == Fate::GonnaDie && HasMeme() ) {
			SpriteCodex::DrawTileBomb( screenPos, gfx );
		}
		SpriteCodex::DrawTileFlag( screenPos, gfx );
		break;
	case State::Revealed:
		if ( HasMeme() ) {
			SpriteCodex::DrawTileBombRed( screenPos, gfx );
		}
		else {
			SpriteCodex::DrawTileNum( screenPos, numAdjacentMemes, gfx );
		}
	}

}

void MemeField::Tile::reveal()
{
	assert( state == State::Hidden );
	state = State::Revealed;
}

void MemeField::Tile::toggleFlag()
{
	assert( state != State::Revealed );
	if ( state == State::Flagged ) {
		state = State::Hidden;
	}
	else {
		state = State::Flagged;
	}
}

void MemeField::Tile::spawnMeme()
{
	assert( hasMeme == false );
	hasMeme = true;
}

void MemeField::Tile::setNumAdjacentMemes(int num)
{
	assert( num != -1 );
	numAdjacentMemes = num;
}

MemeField::MemeField( Vei2 center, int nMemes )
	:
	board( RectI::FromCenter( center, (width * SpriteCodex::tileSize)/2, (height * SpriteCodex::tileSize)/2 ) ),
	totalMemes( nMemes )
{
	assert(nMemes < ( width * height ));

	std::random_device rd;
	std::mt19937 rng( rd() );
	std::uniform_int_distribution<int> xDist( 0, width );
	std::uniform_int_distribution<int> yDist( 0, height );

	Vei2 pos;
	for ( int i = 0; i < nMemes; i++ ) {
		
		do {
			pos.x = xDist( rng );
			pos.y = yDist( rng );
		} while ( tiles[ GridToIndex( pos ) ].HasMeme() );
		tiles[ GridToIndex( pos ) ].spawnMeme();
	}

	for ( pos.y = 0; pos.y < height; pos.y++ ) {
		for ( pos.x = 0; pos.x < width; pos.x++ ) {
			const int count = countAdjacentMemes( pos );
			tiles[ GridToIndex( pos ) ].setNumAdjacentMemes( count );
		}
	}
	
}

RectI MemeField::getRect() const
{
	return board;
}

void MemeField::OnRevealClick( Vei2 screenPos )
{
	if ( playerFate != Fate::YetUncertain ) {
		return;
	}
	assert(IsOnField(screenPos));

	const Vei2 gridPos = ScreenToGrid( screenPos );
	const int index = GridToIndex( gridPos );
	if ( tiles[index].IsHidden() ) {
		tiles[index].reveal();
		if ( tiles[index].HasMeme() ) {
			playerFate = Fate::GonnaDie;
		}
		else if ( tiles[index].HasNoNeighbors() ) {
			const int xStart = std::max<int>(0, gridPos.x - 1);
			const int yStart = std::max<int>(0, gridPos.y - 1);
			const int xEnd = std::min<int>(width - 1, gridPos.x + 1);
			const int yEnd = std::min<int>(height - 1, gridPos.y + 1);

			Vei2 pos;
			int count = 0;
			for (pos.y = yStart; pos.y <= yEnd; pos.y++) {
				for (pos.x = xStart; pos.x <= xEnd; pos.x++) {
					if ( !tiles[GridToIndex(pos)].IsRevealed() ) {
						tiles[GridToIndex(pos)].reveal();
					}
				}
			}
		}
	}
}

void MemeField::OnFlagClick( Vei2 screenPos )
{
	if ( playerFate != Fate::YetUncertain ) {
		return;
	}
	assert( IsOnField( screenPos ) );

	const Vei2 gridPos = ScreenToGrid(screenPos);
	const int index = GridToIndex(gridPos);
	if ( !tiles[index].IsRevealed() ) {
		tiles[index].toggleFlag();
		if ( tiles[index].HasMeme() ) {
			correctMemesFlagged++;

			if ( correctMemesFlagged == totalMemes ) {
				playerFate = Fate::GonnaLive;
			}

		}
	}
}

bool MemeField::IsOnField(Vei2 screenPos) const
{
	return screenPos.x > board.left && screenPos.x < board.right
		&& screenPos.y > board.top && screenPos.y < board.bottom;
}

void MemeField::Draw(Graphics& gfx) const
{
	if ( playerFate == Fate::YetUncertain ) {
		gfx.DrawRect( getRect().GetExpanded(5), Colors::Yellow );
	}
	else if ( playerFate == Fate::GonnaLive ) {
		gfx.DrawRect( getRect().GetExpanded(5), Colors::Green );
	}
	else {
		gfx.DrawRect( getRect().GetExpanded(5), Colors::Red );
	}
	gfx.DrawRect( getRect(), SpriteCodex::baseColor );
	
	Vei2 pos;
	for ( pos.y = 0; pos.y < height; pos.y++ ) {
		for ( pos.x = 0; pos.x < width; pos.x++ ) {
			tiles[ GridToIndex( pos ) ].Draw( GridToScreen( pos ), playerFate, gfx );
		}
	}
}

int MemeField::countAdjacentMemes(Vei2 gridPos) const
{
	const int xStart = std::max<int>( 0, gridPos.x - 1 );
	const int yStart = std::max<int>( 0, gridPos.y - 1 );
	const int xEnd = std::min<int>( width-1, gridPos.x + 1 );
	const int yEnd = std::min<int>( height-1, gridPos.y + 1 );

	Vei2 pos;
	int count = 0;
	for ( pos.y = yStart; pos.y <= yEnd; pos.y++ ) {
		for ( pos.x = xStart; pos.x <= xEnd; pos.x++ ) {
			if ( tiles[GridToIndex( pos ) ].HasMeme() ) {
				count++;
			}
		}
	}
	return count;
}

Vei2 MemeField::GridToScreen(const Vei2& gridPos) const
{
	return gridPos * SpriteCodex::tileSize + Vei2( board.left, board.top );
}

Vei2 MemeField::ScreenToGrid(const Vei2& screenPos) const
{
	return ( screenPos - Vei2(board.left, board.top) ) / SpriteCodex::tileSize;
}

int MemeField::GridToIndex(const Vei2& gridPos) const
{
	return gridPos.y * width + gridPos.x;
}

Vei2 MemeField::IndexToGrid(const int i) const
{
	const int y = i / width;
	const int x = i - ( y * width );

	return Vei2( x, y );
}
