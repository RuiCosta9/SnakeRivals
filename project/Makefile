PROG=proj

SRCS = proj.c app.c state.c menu_state.c play_menu_state.c join_state.c instructions_state.c singleplayer_state.c multiplayer_state.c pause_state.c game_over_state.c renderer.c renderer_helpers.c renderer_menus.c renderer_game.c renderer_overlays.c renderer_multiplayer.c renderer_lobby.c renderer_score_race.c renderer_arena_battle.c hud.c text.c sprite.c assets.c mouse_cursor.c ui_layout.c menu_nav.c snake.c food.c collision.c arena_walls.c powerup.c magnet.c score.c high_score.c serial_queue.c serial.c multiplayer_protocol.c multiplayer_session.c multiplayer_collision.c multiplayer_effects.c multiplayer_session_rules.c multiplayer_session_accessors.c

CFLAGS += -pedantic

CPPFLAGS += -D LCOM_OPTIMIZED

CPPFLAGS += -Isrc
CPPFLAGS += -Isrc/app
CPPFLAGS += -Isrc/states
CPPFLAGS += -Isrc/game
CPPFLAGS += -Isrc/render
CPPFLAGS += -Isrc/ui
CPPFLAGS += -Isrc/multiplayer
CPPFLAGS += -Isrc/resources
CPPFLAGS += -I../lab2
CPPFLAGS += -I../lab3
CPPFLAGS += -I../lab4
CPPFLAGS += -I../lab5

.PATH: src
.PATH: src/app
.PATH: src/states
.PATH: src/game
.PATH: src/render
.PATH: src/ui
.PATH: src/multiplayer
.PATH: src/resources

LAB_LIBS = ../lab5/libvideo.a ../lab4/libmouse.a ../lab3/libkbd.a ../lab2/libtimer.a

DPADD += ${LAB_LIBS} ${LIBLCF}
LDADD += -L../lab5 -lvideo -L../lab4 -lmouse -L../lab3 -lkbd -L../lab2 -ltimer -llcf

../lab2/libtimer.a: ../lab2/timer.c ../lab2/utils.c ../lab2/timer.h ../lab2/i8254.h
	${MAKE} -C ../lab2 lib

../lab3/libkbd.a: ../lab3/kbc.c ../lab3/kbc.h ../lab3/i8042.h ../lab2/libtimer.a
	${MAKE} -C ../lab3 lib

../lab4/libmouse.a: ../lab4/mouse.c ../lab4/mouse.h ../lab4/i8042.h ../lab2/libtimer.a
	${MAKE} -C ../lab4 lib

../lab5/libvideo.a: ../lab5/video.c ../lab5/video.h ../lab5/vbe.c ../lab5/vbe.h ../lab5/vbe_defs.h
	${MAKE} -C ../lab5 lib

.include <minix.lcom.mk>
