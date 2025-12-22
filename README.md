# Jazz in Sea

![important](assets/important.jpg)

An open source Cez engine written completely in C.

## License

This software is licensed under the GPLv3.

## What's Cez?

Cez is a turn based board game, like chess but only played with pawns and knights. The main objective of the game is to move all your remaining pieces to the center of the board or to capture all of the opponent's pieces.

As one side loses their pieces, they have less pieces to get to the center. However, it is also important to remember that losing all of them is a defeat. Because of this, both sides try to balance the position and count of their pieces. For example, having 5 pawns against 2 pawns would be very advantageous in chess, but in Cez the case is usually the opposite -of course depending on the piece positions-.

The pawns move one square either horizontally or vertically. To capture a piece, pawn jumps over that piece and moves to the next square (moving two squares in total) just like in checkers. However in Cez, multiple captures can not be made in the same turn.

The knights move two spaces, ignoring the piece one square away from them. To capture, they jump over the piece and move to the next square (moving three squares in total).

If there are available capture moves on the board, the player must make a capture move. If there are more than one available captures, any of the capture moves can be chosen.

To win the game, the player must put at least one piece on the center squares (D4, D5, E4, E5). These pieces are called _centered_ pieces. Any other piece that is horizontally or vertically adjacent to a _centered_ piece is also a _centered_ piece. The goal is to make all pieces centered.

Like in chess, if any position is repeated 3 times the game ends in a draw. If both players make all their pieces centered at the same time, the game also ends in a draw.

## Installing from source

First build the binary,

    $ make build

To build with debug information available, use this instead:

    $ make debug

Now install the binary,

    # make install

To uninstall,

    # make uninstall

## Usage

Running the executable without any command line option arguments will start interactive mode, which allows running different commands in a shell like environment. Here, running the command `help` will print information about the commands.

It is also possible to run these commands directly on the command line. Directly passing the command as an argument will cause it to first run this command and then start interactive mode.

For more information about all of the options, run with `-h`.

## Notations:

### Squares:

    <square> = <file_letter> <rank_number>

### Moves:

    <regular_move> = <starting_square> '>' <ending_file_or_rank>
    <capture_move> = <starting_square> 'x' <ending_file_or_rank>
    <list_of_moves> = '{' ' ' (<move> ' ')* '}'

### Board FEN:

    <row_fen> = [1-8PNpn]{1,8}
    <fen> = (<row_fen> '|'){7} <row_fen> ' ' ('w' | 'b')
