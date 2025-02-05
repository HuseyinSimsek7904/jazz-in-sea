# Jazz in Sea

![important](assets/important.jpg)

An open source Cez engine written completely in C.

## License

This software is licensed under the GPLv3.

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
