/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"
#include "common/util.h"
#include "common/events.h"

#include "graphics/cursorman.h"

#include "made/made.h"
#include "made/resource.h"
#include "made/database.h"
#include "made/screen.h"
#include "made/script.h"
#include "made/pmvplayer.h"
#include "made/scriptfuncs.h"
#include "made/music.h"

namespace Made {

/* ScriptFunctionsRtz */

typedef Common::Functor2Mem<int16, int16*, int16, ScriptFunctionsRtz> ExternalFuncRtz;
#define External(x) \
	_externalFuncs.push_back(new ExternalFuncRtz(this, &ScriptFunctionsRtz::x));  \
	_externalFuncNames.push_back(#x);
void ScriptFunctionsRtz::setupExternalsTable() {

	External(o1_SYSTEM);
	External(o1_INITGRAF);
	External(o1_RESTOREGRAF);
	External(o1_DRAWPIC);
	External(o1_CLS);
	External(o1_SHOWPAGE);
	External(o1_EVENT);
	External(o1_EVENTX);
	External(o1_EVENTY);
	External(o1_EVENTKEY);
	External(o1_VISUALFX);
	External(o1_PLAYSND);
	External(o1_PLAYMUS);
	External(o1_STOPMUS);
	External(o1_ISMUS);
	External(o1_TEXTPOS);
	External(o1_FLASH);
	External(o1_PLAYNOTE);
	External(o1_STOPNOTE);
	External(o1_PLAYTELE);
	External(o1_STOPTELE);
	External(o1_HIDECURS);
	External(o1_SHOWCURS);
	External(o1_MUSICBEAT);
	External(o1_SCREENLOCK);
	External(o1_ADDSPRITE);
	External(o1_FREEANIM);
	External(o1_DRAWSPRITE);
	External(o1_ERASESPRITES);
	External(o1_UPDATESPRITES);
	External(o1_GETTIMER);
	External(o1_SETTIMER);
	External(o1_RESETTIMER);
	External(o1_ALLOCTIMER);
	External(o1_FREETIMER);
	External(o1_PALETTELOCK);
	External(o1_FONT);
	External(o1_DRAWTEXT);
	External(o1_HOMETEXT);
	External(o1_TEXTRECT);
	External(o1_TEXTXY);
	External(o1_DROPSHADOW);
	External(o1_TEXTCOLOR);
	External(o1_OUTLINE);
	External(o1_LOADCURSOR);
	External(o1_SETGROUND);
	External(o1_RESTEXT);
	External(o1_CLIPAREA);
	External(o1_SETCLIP);
	External(o1_ISSND);
	External(o1_STOPSND);
	External(o1_PLAYVOICE);
	External(o1_CDPLAY);
	External(o1_STOPCD);
	External(o1_CDSTATUS);
	External(o1_CDTIME);
	External(o1_CDPLAYSEG);
	External(o1_PRINTF);
	External(o1_MONOCLS);
	External(o1_SNDENERGY);
	External(o1_CLEARTEXT);
	External(o1_ANIMTEXT);
	External(o1_TEXTWIDTH);
	External(o1_PLAYMOVIE);
	External(o1_LOADSND);
	External(o1_LOADMUS);
	External(o1_LOADPIC);
	External(o1_MUSICVOL);
	External(o1_RESTARTEVENTS);
	External(o1_PLACESPRITE);
	External(o1_PLACETEXT);
	External(o1_DELETECHANNEL);
	External(o1_CHANNELTYPE);
	External(o1_SETSTATE);
	External(o1_SETLOCATION);
	External(o1_SETCONTENT);
	External(o1_EXCLUDEAREA);
	External(o1_SETEXCLUDE);
	External(o1_GETSTATE);
	External(o1_PLACEANIM);
	External(o1_SETFRAME);
	External(o1_GETFRAME);
	External(o1_GETFRAMECOUNT);
	External(o1_PICWIDTH);
	External(o1_PICHEIGHT);
	External(o1_SOUNDRATE);
	External(o1_DRAWANIMPIC);
	External(o1_LOADANIM);
	External(o1_READTEXT);
	External(o1_READMENU);
	External(o1_DRAWMENU);
	External(o1_MENUCOUNT);
	External(o1_SAVEGAME);
	External(o1_LOADGAME);
	External(o1_GAMENAME);
	External(o1_SHAKESCREEN);
	External(o1_PLACEMENU);
	External(o1_SETVOLUME);
	External(o1_WHATSYNTH);
	External(o1_SLOWSYSTEM);

}
#undef External

int16 ScriptFunctionsRtz::o1_SYSTEM(int16 argc, int16 *argv) {
	// This opcode is empty.
	return 0;
}

int16 ScriptFunctionsRtz::o1_INITGRAF(int16 argc, int16 *argv) {
	// This opcode is empty.
	return 0;
}

int16 ScriptFunctionsRtz::o1_RESTOREGRAF(int16 argc, int16 *argv) {
	// This opcode is empty.
	return 0;
}

int16 ScriptFunctionsRtz::o1_DRAWPIC(int16 argc, int16 *argv) {
	return _vm->_screen->drawPic(argv[4], argv[3], argv[2], argv[1], argv[0]);
}

int16 ScriptFunctionsRtz::o1_CLS(int16 argc, int16 *argv) {
 	_vm->_screen->clearScreen();
	return 0;
}

int16 ScriptFunctionsRtz::o1_SHOWPAGE(int16 argc, int16 *argv) {
	_vm->_screen->show();
	return 0;
}

int16 ScriptFunctionsRtz::o1_EVENT(int16 argc, int16 *argv) {

	Common::Event event;
	Common::EventManager *eventMan = g_system->getEventManager();

	int16 eventNum = 0;

	if (eventMan->pollEvent(event)) {
		switch (event.type) {

		case Common::EVENT_MOUSEMOVE:
			_vm->_eventMouseX = event.mouse.x;
			_vm->_eventMouseY = event.mouse.y;
			break;
			
		case Common::EVENT_LBUTTONDOWN:
			eventNum = 1;
			break;

		/*
		case Common::EVENT_LBUTTONUP:
			eventNum = 2; // TODO: Is this correct?
			break;
		*/

		case Common::EVENT_RBUTTONDOWN:
			eventNum = 3;
			break;

		/*
		case Common::EVENT_RBUTTONUP:
			eventNum = 4; // TODO: Is this correct?
			break;
		*/

		case Common::EVENT_KEYDOWN:
			_vm->_eventKey = event.kbd.ascii;
			// For unknown reasons, the game accepts ASCII code
			// 9 as backspace
			if (_vm->_eventKey == Common::KEYCODE_BACKSPACE)
				_vm->_eventKey = 9;
			eventNum = 5;
			break;

		case Common::EVENT_QUIT:
			_vm->_quit = true;
			break;

		default:
			break;

		}
	}

	_vm->_system->updateScreen();

	return eventNum;
}

int16 ScriptFunctionsRtz::o1_EVENTX(int16 argc, int16 *argv) {
	return _vm->_eventMouseX;
}

int16 ScriptFunctionsRtz::o1_EVENTY(int16 argc, int16 *argv) {
	return _vm->_eventMouseY;
}

int16 ScriptFunctionsRtz::o1_EVENTKEY(int16 argc, int16 *argv) {
	return _vm->_eventKey;
}

int16 ScriptFunctionsRtz::o1_VISUALFX(int16 argc, int16 *argv) {
	_vm->_screen->setVisualEffectNum(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_PLAYSND(int16 argc, int16 *argv) {
	int soundNum = argv[0];
	bool loop = false;

	if (argc > 1) {
		soundNum = argv[1];
		loop = (argv[0] == 1);
	}

	if (soundNum > 0) {
		if (!_vm->_mixer->isSoundHandleActive(_audioStreamHandle)) {
			_vm->_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_audioStreamHandle, 
										 _vm->_res->getSound(soundNum)->getAudioStream(_vm->_soundRate, loop));
		}
	}

	return 0;
}

int16 ScriptFunctionsRtz::o1_PLAYMUS(int16 argc, int16 *argv) {
	int16 musicNum = argv[0];
	if (musicNum > 0) {
		_xmidiRes = _vm->_res->getXmidi(musicNum);
		if (_xmidiRes)
			_vm->_music->playXMIDI(_xmidiRes);
	}
	return 0;
}

int16 ScriptFunctionsRtz::o1_STOPMUS(int16 argc, int16 *argv) {
	if (_vm->_music->isPlaying() && _xmidiRes) {
		_vm->_music->stop();
		_vm->_res->freeResource(_xmidiRes);
		_xmidiRes = NULL;
	}
	return 0;
}

int16 ScriptFunctionsRtz::o1_ISMUS(int16 argc, int16 *argv) {
	if (_vm->_music->isPlaying())
		return 1;
	else
		return 0;
}

int16 ScriptFunctionsRtz::o1_TEXTPOS(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_TEXTPOS");
	// This seems to be some kind of low-level opcode.
	// The original engine calls int 10h to set the VGA cursor position.
	return 0;
}

int16 ScriptFunctionsRtz::o1_FLASH(int16 argc, int16 *argv) {
	_vm->_screen->flash(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_PLAYNOTE(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_PLAYNOTE");
	return 0;
}

int16 ScriptFunctionsRtz::o1_STOPNOTE(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_STOPNOTE");
	return 0;
}

int16 ScriptFunctionsRtz::o1_PLAYTELE(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_PLAYTELE");
	return 0;
}

int16 ScriptFunctionsRtz::o1_STOPTELE(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_STOPTELE");
	return 0;
}

int16 ScriptFunctionsRtz::o1_HIDECURS(int16 argc, int16 *argv) {
	_vm->_system->showMouse(false);
	return 0;
}

int16 ScriptFunctionsRtz::o1_SHOWCURS(int16 argc, int16 *argv) {
	_vm->_system->showMouse(true);
	return 0;
}

int16 ScriptFunctionsRtz::o1_MUSICBEAT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_MUSICBEAT");
	return 0;
}

int16 ScriptFunctionsRtz::o1_SCREENLOCK(int16 argc, int16 *argv) {
	_vm->_screen->setScreenLock(argv[0] != 0);
	return 0;
}

int16 ScriptFunctionsRtz::o1_ADDSPRITE(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_ADDSPRITE");
	//_vm->_screen->drawSprite(argv[2], argv[1], argv[0]);
	return 0;//argv[2];
}

int16 ScriptFunctionsRtz::o1_FREEANIM(int16 argc, int16 *argv) {
	_vm->_screen->clearChannels();
	return 0;
}

int16 ScriptFunctionsRtz::o1_DRAWSPRITE(int16 argc, int16 *argv) {
	return _vm->_screen->drawSprite(argv[2], argv[1], argv[0]);
}

int16 ScriptFunctionsRtz::o1_ERASESPRITES(int16 argc, int16 *argv) {
	_vm->_screen->clearChannels();
	return 0;
}

int16 ScriptFunctionsRtz::o1_UPDATESPRITES(int16 argc, int16 *argv) {
	_vm->_screen->updateSprites();
	return 0;
}

int16 ScriptFunctionsRtz::o1_GETTIMER(int16 argc, int16 *argv) {
	return _vm->getTimer(argv[0]);
}

int16 ScriptFunctionsRtz::o1_SETTIMER(int16 argc, int16 *argv) {
	_vm->setTimer(argv[1], argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_RESETTIMER(int16 argc, int16 *argv) {
	_vm->resetTimer(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_ALLOCTIMER(int16 argc, int16 *argv) {
	int16 timerNum = _vm->allocTimer();
	return timerNum;
}

int16 ScriptFunctionsRtz::o1_FREETIMER(int16 argc, int16 *argv) {
	_vm->freeTimer(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_PALETTELOCK(int16 argc, int16 *argv) {
	_vm->_screen->setPaletteLock(argv[0] != 0);
	return 0;
}

int16 ScriptFunctionsRtz::o1_FONT(int16 argc, int16 *argv) {
	_vm->_screen->setFont(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_DRAWTEXT(int16 argc, int16 *argv) {
	// TODO: Needs vsprintf to get the correct text
	Object *obj = _vm->_dat->getObject(argv[argc - 1]);
	const char *text = obj->getString();
	_vm->_screen->printText(text);
	return 0;
}

int16 ScriptFunctionsRtz::o1_HOMETEXT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_HOMETEXT");
	return 0;
}

int16 ScriptFunctionsRtz::o1_TEXTRECT(int16 argc, int16 *argv) {
	int16 x1 = CLIP<int16>(argv[4], 1, 318);
	int16 y1 = CLIP<int16>(argv[3], 1, 198);
	int16 x2 = CLIP<int16>(argv[2], 1, 318);
	int16 y2 = CLIP<int16>(argv[1], 1, 198);
	//int16 textValue = argv[0];
	// TODO: textValue
	_vm->_screen->setTextRect(Common::Rect(x1, y1, x2, y2));
	return 0;
}

int16 ScriptFunctionsRtz::o1_TEXTXY(int16 argc, int16 *argv) {
	int16 x = CLIP<int16>(argv[1], 1, 318);
	int16 y = CLIP<int16>(argv[0], 1, 198);
	_vm->_screen->setTextXY(x, y);
	return 0;
}

int16 ScriptFunctionsRtz::o1_DROPSHADOW(int16 argc, int16 *argv) {
	// if the drop shadow color is -1, then text drop shadow is disabled
	// when font drop shadow is enabled, outline is disabled
	_vm->_screen->setDropShadowColor(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_TEXTCOLOR(int16 argc, int16 *argv) {
	_vm->_screen->setTextColor(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_OUTLINE(int16 argc, int16 *argv) {
	// if the outline color is -1, then text outline is disabled
	// when font outline is enabled, drop shadow is disabled
	_vm->_screen->setOutlineColor(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_LOADCURSOR(int16 argc, int16 *argv) {
	PictureResource *flex = _vm->_res->getPicture(argv[2]);
	Graphics::Surface *surf = flex->getPicture();
	CursorMan.replaceCursor((const byte *)surf->pixels, surf->w, surf->h, argv[1], argv[0], 0);
	CursorMan.showMouse(true);
	_vm->_res->freeResource(flex);
	return 0;
}

int16 ScriptFunctionsRtz::o1_SETGROUND(int16 argc, int16 *argv) {
	_vm->_screen->setGround(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_RESTEXT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_RESTEXT");
	return 0;
}

int16 ScriptFunctionsRtz::o1_CLIPAREA(int16 argc, int16 *argv) {
	_vm->_screen->setClipArea(argv[3], argv[2], argv[1], argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_SETCLIP(int16 argc, int16 *argv) {
	_vm->_screen->setClip(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_ISSND(int16 argc, int16 *argv) {
	if (_vm->_mixer->isSoundHandleActive(_audioStreamHandle))
		return 1;
	else
		return 0;
}

int16 ScriptFunctionsRtz::o1_STOPSND(int16 argc, int16 *argv) {
	_vm->_mixer->stopHandle(_audioStreamHandle);
	return 0;
}

int16 ScriptFunctionsRtz::o1_PLAYVOICE(int16 argc, int16 *argv) {
	if (argv[0] > 0) {
		_vm->_mixer->stopHandle(_audioStreamHandle);
		_vm->_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_audioStreamHandle,
			_vm->_res->getSound(argv[0])->getAudioStream(_vm->_soundRate, false));
	}
	return 0;
}

int16 ScriptFunctionsRtz::o1_CDPLAY(int16 argc, int16 *argv) {
	// This one is called loads of times, so it has been commented out to reduce spam
	//warning("Unimplemented opcode: o1_CDPLAY");
	return 0;
}

int16 ScriptFunctionsRtz::o1_STOPCD(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_STOPCD");
	return 0;
}

int16 ScriptFunctionsRtz::o1_CDSTATUS(int16 argc, int16 *argv) {
	// This one is called loads of times, so it has been commented out to reduce spam
	//warning("Unimplemented opcode: o1_CDSTATUS");
	return 0;
}

int16 ScriptFunctionsRtz::o1_CDTIME(int16 argc, int16 *argv) {
	// This one is called loads of times, so it has been commented out to reduce spam
	//warning("Unimplemented opcode: o1_CDTIME");
	return 0;
}

int16 ScriptFunctionsRtz::o1_CDPLAYSEG(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_CDPLAYSEG");
	return 0;
}

int16 ScriptFunctionsRtz::o1_PRINTF(int16 argc, int16 *argv) {
	Object *obj = _vm->_dat->getObject(argv[argc - 1]);
	const char *text = obj->getString();
	debug(4, "--> text = %s", text);
	return 0;
}

int16 ScriptFunctionsRtz::o1_MONOCLS(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_MONOCLS");
	return 0;
}

int16 ScriptFunctionsRtz::o1_SNDENERGY(int16 argc, int16 *argv) {
	// This is called while in-game voices are played
	// Not sure what it's used for
	// -> It's used to animate mouths when NPCs are talking
	// FIXME: This is a workaround for the "sound energy" problem
	// At least the characters move their lips when talking now
	return _vm->_rnd->getRandomNumber(5);
}

int16 ScriptFunctionsRtz::o1_CLEARTEXT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_CLEARTEXT");
	return 1;
}

int16 ScriptFunctionsRtz::o1_ANIMTEXT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_ANIMTEXT");
	return 0;
}

int16 ScriptFunctionsRtz::o1_TEXTWIDTH(int16 argc, int16 *argv) {
	int16 width = 0;
	if (argv[1] > 0) {
		Object *obj = _vm->_dat->getObject(argv[1]);
		const char *text = obj->getString();
		width = _vm->_screen->getTextWidth(argv[0], text);
	}
	return width;
}

int16 ScriptFunctionsRtz::o1_PLAYMOVIE(int16 argc, int16 *argv) {
	const char *movieName = _vm->_dat->getObject(argv[1])->getString();
	_vm->_pmvPlayer->play(movieName);
	return 0;
}

int16 ScriptFunctionsRtz::o1_LOADSND(int16 argc, int16 *argv) {
	SoundResource *sound = _vm->_res->getSound(argv[0]);
	if (sound) {
		_vm->_res->freeResource(sound);
		return 1;
	}
	return 0;
}

int16 ScriptFunctionsRtz::o1_LOADMUS(int16 argc, int16 *argv) {
	GenericResource *xmidi = _vm->_res->getXmidi(argv[0]);
	if (xmidi) {
		_vm->_res->freeResource(xmidi);
		return 1;
	}
	return 0;
}

int16 ScriptFunctionsRtz::o1_LOADPIC(int16 argc, int16 *argv) {
	PictureResource *flex = _vm->_res->getPicture(argv[0]);
	if (flex) {
		_vm->_res->freeResource(flex);
		return 1;
	}
	return 0;
}

int16 ScriptFunctionsRtz::o1_MUSICVOL(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_MUSICVOL");
	return 0;
}

int16 ScriptFunctionsRtz::o1_RESTARTEVENTS(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_RESTARTEVENTS");
	return 0;
}

int16 ScriptFunctionsRtz::o1_PLACESPRITE(int16 argc, int16 *argv) {
	return _vm->_screen->placeSprite(argv[3], argv[2], argv[1], argv[0]);
}

int16 ScriptFunctionsRtz::o1_PLACETEXT(int16 argc, int16 *argv) {
	return _vm->_screen->placeText(argv[6], argv[5], argv[4], argv[3], argv[2], argv[1], argv[0]);
}

int16 ScriptFunctionsRtz::o1_DELETECHANNEL(int16 argc, int16 *argv) {
	_vm->_screen->deleteChannel(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_CHANNELTYPE(int16 argc, int16 *argv) {
	return _vm->_screen->getChannelType(argv[0]);
}

int16 ScriptFunctionsRtz::o1_SETSTATE(int16 argc, int16 *argv) {
	_vm->_screen->setChannelState(argv[1], argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_SETLOCATION(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_SETLOCATION");
	return 0;
}

int16 ScriptFunctionsRtz::o1_SETCONTENT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_SETCONTENT");
	return 0;
}

int16 ScriptFunctionsRtz::o1_EXCLUDEAREA(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_EXCLUDEAREA");
	return 0;
}

int16 ScriptFunctionsRtz::o1_SETEXCLUDE(int16 argc, int16 *argv) {
	_vm->_screen->setExclude(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_GETSTATE(int16 argc, int16 *argv) {
	return _vm->_screen->getChannelState(argv[0]);
}

int16 ScriptFunctionsRtz::o1_PLACEANIM(int16 argc, int16 *argv) {
	return _vm->_screen->placeAnim(argv[4], argv[3], argv[2], argv[1], argv[0]);
}

int16 ScriptFunctionsRtz::o1_SETFRAME(int16 argc, int16 *argv) {
	_vm->_screen->setAnimFrame(argv[1], argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_GETFRAME(int16 argc, int16 *argv) {
	return _vm->_screen->getAnimFrame(argv[0]);
}

int16 ScriptFunctionsRtz::o1_GETFRAMECOUNT(int16 argc, int16 *argv) {
	return _vm->_screen->getAnimFrameCount(argv[0]);
}

int16 ScriptFunctionsRtz::o1_PICWIDTH(int16 argc, int16 *argv) {
	int16 width = 0;
	PictureResource *flex = _vm->_res->getPicture(argv[0]);
	if (flex) {
		width = flex->getPicture()->w;
		_vm->_res->freeResource(flex);
	}
	return width;
}

int16 ScriptFunctionsRtz::o1_PICHEIGHT(int16 argc, int16 *argv) {
	int16 height = 0;
	PictureResource *flex = _vm->_res->getPicture(argv[0]);
	if (flex) {
		height = flex->getPicture()->h;
		_vm->_res->freeResource(flex);
	}
	return height;
}

int16 ScriptFunctionsRtz::o1_SOUNDRATE(int16 argc, int16 *argv) {
	_vm->_soundRate = argv[0];
	return 1;
}

int16 ScriptFunctionsRtz::o1_DRAWANIMPIC(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_DRAWANIMPIC");
	return 0;
}

int16 ScriptFunctionsRtz::o1_LOADANIM(int16 argc, int16 *argv) {
	AnimationResource *anim = _vm->_res->getAnimation(argv[0]);
	if (anim) {
		_vm->_res->freeResource(anim);
		return 1;
	}
	return 0;
}

int16 ScriptFunctionsRtz::o1_READTEXT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_READTEXT");
	return 0;
}

int16 ScriptFunctionsRtz::o1_READMENU(int16 argc, int16 *argv) {
	int16 objectIndex = argv[2];
	int16 menuIndex = argv[1];
	int16 textIndex = argv[0];
	int16 length = 0;
	MenuResource *menu = _vm->_res->getMenu(menuIndex);
	if (menu) {
		const char *text = menu->getString(textIndex);
		debug(4, "objectIndex = %04X; text = %s\n", objectIndex, text);
		Object *obj = _vm->_dat->getObject(objectIndex);
		obj->setString(text);
		_vm->_res->freeResource(menu);
		if (text)
			length = strlen(text);
	}
	return length;
}

int16 ScriptFunctionsRtz::o1_DRAWMENU(int16 argc, int16 *argv) {
	int16 menuIndex = argv[1];
	int16 textIndex = argv[0];
	MenuResource *menu = _vm->_res->getMenu(menuIndex);
	if (menu) {
		const char *text = menu->getString(textIndex);
		if (text)
			_vm->_screen->printText(text);
		_vm->_res->freeResource(menu);
	}
	return 0;
}

int16 ScriptFunctionsRtz::o1_MENUCOUNT(int16 argc, int16 *argv) {
	int16 menuIndex = argv[0];
	int16 count = 0;
	MenuResource *menu = _vm->_res->getMenu(menuIndex);
	if (menu) {
		count = menu->getCount();
		_vm->_res->freeResource(menu);
	}
	return count;
}

int16 ScriptFunctionsRtz::o1_SAVEGAME(int16 argc, int16 *argv) {
	
	int16 saveNum = argv[2];
	int16 descObjectIndex = argv[1];
	int16 version = argv[0];
	
	if (saveNum > 999)
		return 6;

	Object *obj = _vm->_dat->getObject(descObjectIndex);
	const char *description = obj->getString();

	Common::String filename = _vm->getSavegameFilename(saveNum);

	return _vm->_dat->savegame(filename.c_str(), description, version);
	
}

int16 ScriptFunctionsRtz::o1_LOADGAME(int16 argc, int16 *argv) {

	int16 saveNum = argv[1];
	int16 version = argv[0];

	if (saveNum > 999)
		return 1;

	Common::String filename = _vm->getSavegameFilename(saveNum);

	return _vm->_dat->loadgame(filename.c_str(), version);
	
}

int16 ScriptFunctionsRtz::o1_GAMENAME(int16 argc, int16 *argv) {
	
	int16 descObjectIndex = argv[2];
	int16 saveNum = argv[1];
	/*int16 version = argv[0];*/
	Common::String description;

	if (saveNum > 999)
		return 1;

	Common::String filename = _vm->getSavegameFilename(saveNum);

	Object *obj = _vm->_dat->getObject(descObjectIndex);

	if (_vm->_dat->getSavegameDescription(filename.c_str(), description)) {
		obj->setString(description.c_str());
		return 0;
	} else {
		obj->setString("");
		return 1;
	}

}

int16 ScriptFunctionsRtz::o1_SHAKESCREEN(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_SHAKESCREEN");
	return 0;
}

int16 ScriptFunctionsRtz::o1_PLACEMENU(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_PLACEMENU");
	return 0;
}

int16 ScriptFunctionsRtz::o1_SETVOLUME(int16 argc, int16 *argv) {
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, argv[0] * 25);
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, argv[0] * 25);
	return 0;
}

int16 ScriptFunctionsRtz::o1_WHATSYNTH(int16 argc, int16 *argv) {
	// 0 = Default
	// 1 = PCSPKR
	// 2 = SBFM/ADLIB
	// 3 = ADLIBG
	// 4 = MT32MPU
	warning("Unimplemented opcode: o1_WHATSYNTH");
	return 0;
}

int16 ScriptFunctionsRtz::o1_SLOWSYSTEM(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_SLOWSYSTEM");
	return 0;
}

} // End of namespace Made
