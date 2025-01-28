# Jazz in Sea

![important](assets/important.jpg)

An open source Cez engine written completely in C.

## License

This software is licensed under the GPLv3.

## Building from source

Just clone the repo and run `make build`. This will create the executable _main_ under the directory _build_.

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
