# SnakeRivals

Snake Rivals is a Snake game developed in C for Minix 3, as part of the Laboratórios de Computadores (LCOM) course at FEUP.

The game includes a complete singleplayer mode, with food, scoring, temporary power-ups, snake death animation and a game over screen. It also includes a local multiplayer mode between two machines connected through the serial port, with different game modes, independent player states, power-ups, lobby interaction and synchronized gameplay.
Team

    André Brás Pereira — up202405143@up.pt
    Gonçalo José Pereira Pacheco — up202405159@up.pt
    Rui Rocha Oliveira Azeredo Costa — up202405336@up.pt
    Vasco Cabral Rodrigues Cunha Correia — up202405343@up.pt

Features
Singleplayer

    Classic Snake gameplay.

    Food collection and score system.

    Temporary power-ups:
        Double points
        Ghost mode
        Shield
        Magnet
        Frenzy
        Bonus food

    Power-up inventory with current and next item.

    Snake death animation.

    Pause and game over screens.

    High score support.

Multiplayer

    Local multiplayer through serial communication.
    Host and client lobby system.
    Lobby code generation and joining.
    Player connection status.
    Snake color selection for each player.
    Independent food and power-up spawning.
    Score Race mode.
    Arena Battle mode.
    Multiplayer power-up support.
    Winner detection, including score-based tie-breaking.
    Waiting state for the non-host player after a match ends.

Devices used

The project integrates the following devices covered in the LCOM labs:

    Timer, i8254 — controls the game pace, animations and temporary effects.
    Keyboard, KBC — snake movement and menu navigation.
    Mouse, KBC — interaction with menu buttons through the cursor.
    Graphics card, VBE — rendering in graphics mode, using sprites, text and buffering to avoid flickering.
    Serial port, UART — communication between the two machines in multiplayer mode.
    RTC — support for time-based functionality where applicable.

Architecture

The project follows an event-driven structure. Hardware interrupts are handled by the application loop and then dispatched to the current game state.

The code is organized into modules with clear responsibilities:

    project/src/app/ — application initialization, main loop and cleanup.
    project/src/states/ — menu, singleplayer, multiplayer, pause, instructions and game over states.
    project/src/game/ — core game logic, including snake, food, power-ups and reusable mechanics.
    project/src/multiplayer/ — multiplayer session logic, protocol handling, collision rules and effects.
    project/src/render/ — drawing routines, sprites, text and game rendering.
    project/src/resources/ — asset loading and management.
    project/src/ui/ — UI layout and mouse cursor handling.
    lab2/ to lab5/ — reusable drivers from the lab assignments.

How to build and run

Inside Minix, from the repository root:

cd project
make clean
make
lcom_run proj

Multiplayer setup

To play multiplayer, two Minix machines must be connected through the serial port configuration used in the LCOM environment.

One machine should create the lobby as host. The other machine should join using the lobby code shown on the host screen.
Documentation

The project can be documented with Doxygen. The documentation should describe the main modules, public functions, data structures and the overall architecture of the game.

A suggested command is:

cd project
doxygen Doxyfile

The generated HTML documentation is written to project/docs/doxygen/html/index.html.
Course

This project was developed for the Laboratórios de Computadores course, in the 2nd year of the Bachelor in Informatics and Computing Engineering at FEUP.
