/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm/actor.h"
#include "scumm/imuse.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"

#include "common/config-manager.h"
#include "common/timer.h"
#include "common/util.h"

#include "sound/audiocd.h"
#include "sound/mididrv.h"
#include "sound/mixer.h"
#include "sound/mp3.h"
#include "sound/voc.h"
#include "sound/vorbis.h"
#include "sound/flac.h"


namespace Scumm {

struct MP3OffsetTable {					/* Compressed Sound (.SO3) */
	int org_offset;
	int new_offset;
	int num_tags;
	int compressed_size;
};


Sound::Sound(ScummEngine *parent)
	:
	_vm(parent),
	_soundQuePos(0),
	_soundQue2Pos(0),
	_sfxFile(0),
	_offsetTable(0),
	_numSoundEffects(0),
	_soundMode(kVOCMode),
	_talk_sound_a1(0),
	_talk_sound_a2(0),
	_talk_sound_b1(0),
	_talk_sound_b2(0),
	_talk_sound_mode(0),
	_mouthSyncMode(false),
	_endOfMouthSync(false),
	_curSoundPos(0),
	_overrideFreq(0),
	_currentCDSound(0),
	_currentMusic(0),
	_soundsPaused(false),
	_sfxMode(0) {
	
	memset(_soundQue, 0, sizeof(_soundQue));
	memset(_soundQue2Sound, 0, sizeof(_soundQue2Sound));
	memset(_soundQue2Offset, 0, sizeof(_soundQue2Offset));
	memset(_mouthSyncTimes, 0, sizeof(_mouthSyncTimes));
}

Sound::~Sound() {
	stopCDTimer();
	delete _sfxFile;
}

void Sound::addSoundToQueue(int sound, int offset) {
	_vm->VAR(_vm->VAR_LAST_SOUND) = sound;
	// Music resources are in separate file
	if (!((_vm->_heversion >= 70) && sound >= 4000))
		_vm->ensureResourceLoaded(rtSound, sound);
	addSoundToQueue2(sound, offset);
}

void Sound::addSoundToQueue2(int sound, int offset) {
	if ((_vm->_features & GF_HUMONGOUS) && _soundQue2Pos) {
		int i = _soundQue2Pos;
		while (i--) {
			if (_soundQue2Sound[i] == sound)
				return;
		}
	}

	assert(_soundQue2Pos < ARRAYSIZE(_soundQue2Sound));
	_soundQue2Sound[_soundQue2Pos] = sound;
	_soundQue2Offset[_soundQue2Pos] = offset;
	_soundQue2Pos++;
}

void Sound::processSoundQues() {
	int i = 0, num, offset, snd;
	int data[16];

	processSfxQueues();

	if (_vm->_features & GF_DIGI_IMUSE)
		return;

	while (_soundQue2Pos) {
		_soundQue2Pos--;
		snd = _soundQue2Sound[_soundQue2Pos];
		offset = _soundQue2Offset[_soundQue2Pos];
		if (snd)
			playSound(snd, offset);
	}

	while (i < _soundQuePos) {
		num = _soundQue[i++];
		if (i + num > _soundQuePos) {
			warning("processSoundQues: invalid num value");
			break;
		}
		memset(data, 0, sizeof(data));
		if (num > 0) {
			for (int j = 0; j < num; j++)
				data[j] = _soundQue[i + j];
			i += num;

			debugC(DEBUG_IMUSE, "processSoundQues(%d,%d,%d,%d,%d,%d,%d,%d,%d)",
						data[0] >> 8, data[0] & 0xFF,
						data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

			if (_vm->_imuse) {
				_vm->VAR(_vm->VAR_SOUNDRESULT) = (short)_vm->_imuse->doCommand (num, data);
			}
		}
	}
	_soundQuePos = 0;
}

void Sound::setOverrideFreq(int freq) {
	_overrideFreq = freq;
}

void Sound::playSound(int soundID, int offset) {
	byte *ptr;
	char *sound;
	int size = -1;
	int rate;
	byte flags = SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE;
	
	debugC(DEBUG_SOUND, "playSound #%d (room %d)", soundID, 
		_vm->getResourceRoomNr(rtSound, soundID));

	if ((_vm->_heversion >= 70) && soundID >= 4000) {
		int music_offs, total_size;
		char buf[32];
		File musicFile;
		sprintf(buf, "%s.he4", _vm->getGameName());
		if (musicFile.open(buf) == false) {
			warning("playSound: Music file is not open");
			return;
		}
		musicFile.seek(4, SEEK_SET);
		total_size = musicFile.readUint32BE();

		// Skip header junk
		musicFile.seek(+20, SEEK_CUR);

		// Skip to correct music header
		uint skip = (soundID - 4001) * 25;
		musicFile.seek(+skip, SEEK_CUR);

		// Skip to offsets
		musicFile.seek(+21, SEEK_CUR);

		music_offs = musicFile.readUint32LE();
		size = musicFile.readUint32LE();

		if (music_offs > total_size || (size + music_offs) > total_size)
			error("Bad music offsets");

		musicFile.seek(music_offs, SEEK_SET);
		ptr = (byte *) calloc(size, 1);
		musicFile.read(ptr, size);
		musicFile.close();

		if (_vm->_heversion == 70) {
			// Allocate a sound buffer, copy the data into it, and play
			sound = (char *)malloc(size);
			memcpy(sound, ptr, size);
			free(ptr);
			_currentMusic = soundID;
			_vm->_mixer->stopHandle(_musicChannelHandle);
			_vm->_mixer->playRaw(&_musicChannelHandle, sound, size, 11025, flags, soundID);
			return;
		}
	} else 
		ptr = _vm->getResourceAddress(rtSound, soundID);

	if (!ptr) {
		return;
	}

	// Support for SFX in Monkey Island 1, Mac version
	// This is rather hackish right now, but works OK. SFX are not sounding
	// 100% correct, though, not sure right now what is causing this.
	else if (READ_UINT32(ptr) == MKID('Mac1')) {

		// Read info from the header
		size = READ_BE_UINT32(ptr+0x60);
		rate = READ_BE_UINT16(ptr+0x64);

		// Skip over the header (fixed size)
		ptr += 0x72;

		// Allocate a sound buffer, copy the data into it, and play
		sound = (char *)malloc(size);
		memcpy(sound, ptr, size);
		_vm->_mixer->playRaw(NULL, sound, size, rate, flags, soundID);
	}
	// Support for Putt-Putt sounds - very hackish, too 8-)
	else if (READ_UINT32(ptr) == MKID('DIGI')) {
		// TODO - discover what data the first chunk, HSHD, contains
		// it might be useful here.
		rate = READ_LE_UINT16(ptr + 22);

		ptr += 8 + READ_BE_UINT32(ptr+12);
		if (READ_UINT32(ptr) != MKID('SDAT'))
			return;	// abort

		size = READ_BE_UINT32(ptr+4) - offset - 8;
		if (_overrideFreq) {
			// Used by the piano in Fatty Bear's Birthday Surprise
			rate = _overrideFreq;
			_overrideFreq = 0;
		}

		// Allocate a sound buffer, copy the data into it, and play
		sound = (char *)malloc(size);
		memcpy(sound, ptr + offset + 8, size);
		_vm->_mixer->playRaw(NULL, sound, size, rate, flags, soundID);
	}
	else if (READ_UINT32(ptr) == MKID('MRAW')) {
		// pcm music in 3DO humongous games
		ptr += 8 + READ_BE_UINT32(ptr+12);
		if (READ_UINT32(ptr) != MKID('SDAT'))
			return;

		size = READ_BE_UINT32(ptr+4) - 8;
		rate = 22050;
		flags = SoundMixer::FLAG_AUTOFREE;

		// Allocate a sound buffer, copy the data into it, and play
		sound = (char *)malloc(size);
		memcpy(sound, ptr + 8, size);
		_currentMusic = soundID;
		_vm->_mixer->stopHandle(_musicChannelHandle);
		_vm->_mixer->playRaw(&_musicChannelHandle, sound, size, rate, flags, soundID);
	}	
	// Support for sampled sound effects in Monkey Island 1 and 2
	else if (READ_UINT32(ptr) == MKID('SBL ')) {
		debug(2, "Using SBL sound effect");
		
		// SBL resources essentially contain VOC sound data.
		// There are at least two main variants: in one,
		// there are two subchunks AUhd and AUdt, in the other
		// the chunks are called WVhd and WVdt. Besides that,
		// the two variants seem pretty similiar.
		
		// The first subchunk (AUhd resp. WVhd) seems to always
		// contain three bytes (00 00 80) of unknown meaning.
		// After that, a second subchunk contains VOC data.
		// Two real examples:
		//
		// 53 42 4c 20 00 00 11 ae  |SBL ....|
		// 41 55 68 64 00 00 00 03  |AUhd....|
		// 00 00 80 41 55 64 74 00  |...AUdt.|
		// 00 11 9b 01 96 11 00 a6  |........|
		// 00 7f 7f 7e 7e 7e 7e 7e  |...~~~~~|
		// 7e 7f 7f 80 80 7f 7f 7f  |~.......|
		// 7f 80 80 7f 7e 7d 7d 7e  |....~}}~|
		// 7e 7e 7e 7e 7e 7e 7e 7f  |~~~~~~~.|
		//
		// And from the non-interactive Sam & Max demo:
		//
		// 53 42 4c 20 00 01 15 6e  |SBL ...n|
		// 57 56 68 64 00 00 00 03  |WVhd....|
		// 00 00 80 57 56 64 74 00  |...WVdt.|
		// 01 15 5b 01 56 15 01 a6  |..[.V...|
		// 00 80 80 80 80 80 80 80  |........|
		// 80 80 80 80 80 80 80 80  |........|
		// 80 80 80 80 80 80 80 80  |........|
		// 80 80 80 80 80 80 80 80  |........|

		size = READ_BE_UINT32(ptr + 4) - 27;
		ptr += 27;

		// Fingolfin says: after eyeballing a single SEGA
		// SBL resource, it would seem as if the content of the
		// data subchunk (AUdt) is XORed with 0x16. At least
		// then a semi-sane VOC header is revealed, with
		// a sampling rate of ~25000 Hz (does that make sense?).
		// I'll add some code to test that theory for now.

		// Check if the resource has already been demangled
		if ((_vm->_gameId == GID_MONKEY_SEGA) && (ptr[0] != 1))	{
			for (int i = 0; i < size; i++)   {
				ptr[i] ^= 0x16;
				if (ptr[i] >= 0x7F)   {
				  ptr[i] = 0xFE - ptr[i];
				  ptr[i] ^= 0x80;
				}
			}
		}
		
		// TODO: It would be nice if we could use readVOCFromMemory() here.
		// We'd have to add the 'Creative Voice File' header for this, though,
		// or make readVOCFromMemory() less strict.

		VocBlockHeader &voc_block_hdr = *(VocBlockHeader *)ptr;
		assert(voc_block_hdr.blocktype == 1);
		size = voc_block_hdr.size[0] + (voc_block_hdr.size[1] << 8) + (voc_block_hdr.size[2] << 16) - 2;
		rate = getSampleRateFromVOCRate(voc_block_hdr.sr);
		assert(voc_block_hdr.pack == 0);

		// Allocate a sound buffer, copy the data into it, and play
		sound = (char *)malloc(size);
		memcpy(sound, ptr + 6, size);
		_vm->_mixer->playRaw(NULL, sound, size, rate, flags, soundID);
	}
	else if ((_vm->_features & GF_FMTOWNS) || READ_UINT32(ptr) == MKID('SOUN') || READ_UINT32(ptr) == MKID('TOWS')) {

		bool tows = READ_UINT32(ptr) == MKID('TOWS');
		if (_vm->_features & GF_FMTOWNS) {
			size = READ_LE_UINT32(ptr);
		} else {
			size = READ_BE_UINT32(ptr + 4) - 2;
			if (tows)
				size += 8;
			ptr += 2;
		}

		rate = 11025;
		int type = *(ptr + 0x0D);
		int numInstruments;

		if (tows)
			type = 0;

		switch (type) {
		case 0:	// Sound effect
			numInstruments = *(ptr + 0x14);
			if (tows)
				numInstruments = 1;
			ptr += 0x16;
			size -= 0x16;

			while (numInstruments--) {
				int waveSize = READ_LE_UINT32(ptr + 0x0C);
				int loopStart = READ_LE_UINT32(ptr + 0x10) * 2;
				int loopEnd = READ_LE_UINT32(ptr + 0x14) - 1;
				rate = READ_LE_UINT32(ptr + 0x18) * 1000 / 0x62;
				ptr += 0x20;
				size -= 0x20;
				if (size < waveSize) {
					warning("Wrong wave size in sound #%i: %i", soundID, waveSize);
					waveSize = size;
				}
				sound = (char *)malloc(waveSize);
				for (int x = 0; x < waveSize; x++) {
					int b = *ptr++;
					if (b < 0x80)
						sound[x] = 0x7F - b;
					else
						sound[x] = b;
				}
				size -= waveSize;

				if (loopEnd > 0)
					flags |= SoundMixer::FLAG_LOOP;

				_vm->_mixer->playRaw(NULL, sound, waveSize, rate, flags, soundID, 255, 0, loopStart, loopEnd);
			}
			break;
		case 1:
		case 255:	// 255 is the type used in Indy3 FMTowns
			// Music (Euphony format)
			if (_vm->_musicEngine)
				_vm->_musicEngine->startSound(soundID);
			break;
		case 2: // CD track resource
			ptr += 0x16;

			if (soundID == _currentCDSound)
				if (pollCD() == 1)
					return;

			{
				int track = ptr[0];
				int loops = ptr[1];
				int start = (ptr[2] * 60 + ptr[3]) * 75 + ptr[4];
				int end = (ptr[5] * 60 + ptr[6]) * 75 + ptr[7];

				playCDTrack(track, loops == 0xff ? -1 : loops, start, end <= start ? 0 : end - start);
			}

			_currentCDSound = soundID;
			break;
		default: // Unsupported sound type
			warning("Unsupported sound sub-type %d", type);
			break;
		}
	}
	else if ((_vm->_gameId == GID_LOOM) && (_vm->_features & GF_MACINTOSH))  {
		// Mac version of Loom uses yet another sound format
		/*
		playSound #9 (room 70)
		000000: 55 00 00 45  73 6f 00 64  01 00 00 00  00 00 00 00   |U..Eso.d........|
		000010: 00 05 00 8e  2a 8f 2d 1c  2a 8f 2a 8f  2d 1c 00 28   |....*.-.*.*.-..(|
		000020: 00 31 00 3a  00 43 00 4c  00 01 00 00  00 01 00 64   |.1.:.C.L.......d|
		000030: 5a 00 01 00  00 00 01 00  64 00 00 01  00 00 00 01   |Z.......d.......|
		000040: 00 64 5a 00  01 00 00 00  01 00 64 5a  00 01 00 00   |.dZ.......dZ....|
		000050: 00 01 00 64  00 00 00 00  00 00 00 07  00 00 00 64   |...d...........d|
		000060: 64 00 00 4e  73 6f 00 64  01 00 00 00  00 00 00 00   |d..Nso.d........|
		000070: 00 05 00 89  3d 57 2d 1c  3d 57 3d 57  2d 1c 00 28   |....=W-.=W=W-..(|
		playSound #16 (room 69)
		000000: dc 00 00 a5  73 6f 00 64  01 00 00 00  00 00 00 00   |....so.d........|
		000010: 00 05 00 00  2a 8f 03 e8  03 e8 03 e8  03 e8 00 28   |....*..........(|
		000020: 00 79 00 7f  00 85 00 d6  00 01 00 00  00 19 01 18   |.y..............|
		000030: 2f 00 18 00  01 18 32 00  18 00 01 18  36 00 18 00   |/.....2.....6...|
		000040: 01 18 3b 00  18 00 01 18  3e 00 18 00  01 18 42 00   |..;.....>.....B.|
		000050: 18 00 01 18  47 00 18 00  01 18 4a 00  18 00 01 18   |....G.....J.....|
		000060: 4e 00 10 00  01 18 53 00  10 00 01 18  56 00 10 00   |N.....S.....V...|
		000070: 01 18 5a 00  10 00 02 28  5f 00 01 00  00 00 00 00   |..Z....(_.......|
		*/
	}
	else if ((_vm->_features & GF_MACINTOSH) && (_vm->_gameId == GID_INDY3) && (ptr[26] == 0)) {
		size = READ_BE_UINT16(ptr + 12);
		rate = 3579545 / READ_BE_UINT16(ptr + 20);
		sound = (char *)malloc(size);
		int vol = ptr[24] * 4;
		memcpy(sound,ptr + READ_BE_UINT16(ptr + 8), size);
		_vm->_mixer->playRaw(NULL, sound, size, rate, SoundMixer::FLAG_AUTOFREE, soundID, vol, 0);
	}
	else {
		
		if (_vm->_gameId == GID_MONKEY_VGA || _vm->_gameId == GID_MONKEY_EGA
			|| (_vm->_gameId == GID_MONKEY && _vm->_features & GF_MACINTOSH)) {
			// Sound is currently not supported at all in the amiga versions of these games
			if (_vm->_features & GF_AMIGA) {
				int track = -1;
				if (soundID == 50)
					track = 17;
				else if (ptr[6] == 0x7F && ptr[7] == 0x00 && ptr[8] == 0x80)
				{
					char tracks[16] = {13,14,10,3,4,9,16,5,1,8,2,15,6,7,11,12};
					if (ptr[9] == 0x0E)
						track = 18;
					else	track = tracks[ptr[9] - 0x23];
				}
				if (track != -1) {
					playCDTrack(track,((track < 5) || (track > 16)) ? 1 : -1,0,0);
					stopCDTimer();
					_currentCDSound = soundID;
				}
				return;
			}
	
			// Works around the fact that in some places in MonkeyEGA/VGA,
			// the music is never explicitly stopped.
			// Rather it seems that starting a new music is supposed to
			// automatically stop the old song.
			if (_vm->_imuse) {
				if (READ_UINT32(ptr) != MKID('ASFX'))
					_vm->_imuse->stopAllSounds();
			}
		}
	
		if (_vm->_musicEngine) {
			_vm->_musicEngine->startSound(soundID);
		}
	}
}

void Sound::processSfxQueues() {

	if (_talk_sound_mode != 0) {
		if (_talk_sound_mode & 1)
			startTalkSound(_talk_sound_a1, _talk_sound_b1, 1);
		if (_talk_sound_mode & 2)
			startTalkSound(_talk_sound_a2, _talk_sound_b2, 2, &_talkChannelHandle);
		_talk_sound_mode = 0;
	}

	const int act = _vm->getTalkingActor();
	if ((_sfxMode & 2) && act != 0) {
		Actor *a;
		bool finished;

		if (_vm->_imuseDigital) {
			finished = !isSoundRunning(kTalkSoundID);
		} else {
			finished = !_talkChannelHandle.isActive();
		}

		if ((uint) act < 0x80 && ((_vm->_version == 8) || (_vm->_version <= 7 && !_vm->_string[0].no_talk_anim))) {
			a = _vm->derefActor(act, "processSfxQueues");
			if (a->isInCurrentRoom()) {
				if (isMouthSyncOff(_curSoundPos) && !_mouthSyncMode) {
					if (!_endOfMouthSync)
						a->runActorTalkScript(a->talkStopFrame);
					_mouthSyncMode = 0;
				} else  if (isMouthSyncOff(_curSoundPos) == 0 && !_mouthSyncMode) {
					a->runActorTalkScript(a->talkStartFrame);
					_mouthSyncMode = 1;
				}

				if (_vm->_version <= 6 && finished)
					a->runActorTalkScript(a->talkStopFrame);
			}
		}

		if ((!ConfMan.getBool("subtitles") && finished && _vm->_version <= 6) || (finished && _vm->_talkDelay == 0)) {
			if (!(_vm->_version == 8 && _vm->VAR(_vm->VAR_HAVE_MSG) == 0))
				_vm->stopTalk();
		}
	}

	if (_sfxMode & 1) {
		if (isSfxFinished()) {
			_sfxMode &= ~1;
		}
	}
}

static int compareMP3OffsetTable(const void *a, const void *b) {
	return ((const MP3OffsetTable *)a)->org_offset - ((const MP3OffsetTable *)b)->org_offset;
}

void Sound::startTalkSound(uint32 offset, uint32 b, int mode, PlayingSoundHandle *handle) {
	int num = 0, i;
	int size = 0;
	byte *sound;
	int id = -1;

	if (_vm->_gameId == GID_CMI) {
		_sfxMode |= mode;
		return;
	} else if (_vm->_gameId == GID_DIG) {
		_sfxMode |= mode;
		if (!(_vm->_features & GF_DEMO))
			return;

		char filename[30];
		char roomname[10];

		if (offset == 1)
			strcpy(roomname, "logo");
		else if (offset == 15)
			strcpy(roomname, "canyon");
		else if (offset == 17)
			strcpy(roomname, "pig");
		else if (offset == 18)
			strcpy(roomname, "derelict");
		else if (offset == 19)
			strcpy(roomname, "wreck");
		else if (offset == 20)
			strcpy(roomname, "grave");
		else if (offset == 23)
			strcpy(roomname, "nexus");
		else if (offset == 79)
			strcpy(roomname, "newton");
		else {
			warning("startTalkSound: dig demo: unknown room number: %d", offset);
			return;
		}

		_sfxFile->close();
		sprintf(filename, "audio/%s.%d/%d.voc", roomname, offset, b);
		_sfxFile->open(filename);
		if (!_sfxFile->isOpen()) {
			sprintf(filename, "%d.%d.voc", offset, b);
			_sfxFile->open(filename);
		}
		if (!_sfxFile->isOpen()) {
			warning("startTalkSound: dig demo: voc file not found");
			return;
		}
	} else {

		if (!_sfxFile->isOpen()) {
			warning("startTalkSound: SFX file is not open");
			return;
		}

		if (_vm->_features & GF_HUMONGOUS) {
			_sfxMode |= mode;

			// SKIP TALK (8) HSHD (14)
			_sfxFile->seek(offset + 22, SEEK_SET);
			int rate = _sfxFile->readUint16LE();
			// SKIP HSHD (8) and SDAT (8)
			_sfxFile->seek(+16, SEEK_CUR);

			size = b - 40;
			sound = (byte *)malloc(size);
			_sfxFile->read(sound, size);
			_vm->_mixer->playRaw(handle, sound, size, rate, SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
			return;
		}

		// Some games frequently assume that starting one sound effect will
		// automatically stop any other that may be playing at that time. So
		// that is what we do here, but we make an exception for speech.
		//
		// Do any other games than these need this hack?
		//
		// HACK: Checking for script 99 in Sam & Max is to keep Conroy's song
		// from being interrupted.

		if (mode == 1 && (_vm->_gameId == GID_TENTACLE
			|| (_vm->_gameId == GID_SAMNMAX && !_vm->isScriptRunning(99)))) {
			id = 777777;
			_vm->_mixer->stopID(id);
		}

		if (b > 8) {
			num = (b - 8) >> 1;
		}

		if (_offsetTable != NULL) {
			MP3OffsetTable *result = NULL, key;
	
			key.org_offset = offset;
			result = (MP3OffsetTable *)bsearch(&key, _offsetTable, _numSoundEffects,
													sizeof(MP3OffsetTable), compareMP3OffsetTable);

			if (result == NULL) {
				warning("startTalkSound: did not find sound at offset %d !", offset);
				return;
			}
			if (2 * num != result->num_tags) {
				warning("startTalkSound: number of tags do not match (%d - %d) !", b,
								result->num_tags);
				num = result->num_tags;
			}
			offset = result->new_offset;
			size = result->compressed_size;
		} else {
			offset += 8;
			size = -1;
		}

		_sfxFile->seek(offset, SEEK_SET);

		assert(num + 1 < (int)ARRAYSIZE(_mouthSyncTimes));
		for (i = 0; i < num; i++)
			_mouthSyncTimes[i] = _sfxFile->readUint16BE();
	
		_mouthSyncTimes[i] = 0xFFFF;
		_sfxMode |= mode;
		_curSoundPos = 0;
		_mouthSyncMode = true;
	}

	if (!_soundsPaused && _vm->_mixer->isReady())
		startSfxSound(_sfxFile, size, handle, id);
}

void Sound::stopTalkSound() {
	if (_sfxMode & 2) {
		if (_vm->_imuseDigital) {
			_vm->_imuseDigital->stopSound(kTalkSoundID);
		} else {
			_vm->_mixer->stopHandle(_talkChannelHandle);
		}
		_sfxMode &= ~2;
	}
}

bool Sound::isMouthSyncOff(uint pos) {
	uint j;
	bool val = true;
	uint16 *ms = _mouthSyncTimes;

	_endOfMouthSync = false;
	do {
		val = !val;
		j = *ms++;
		if (j == 0xFFFF) {
			_endOfMouthSync = true;
			break;
		}
	} while (pos > j);
	return val;
}


int Sound::isSoundRunning(int sound) const {

	if (_vm->_imuseDigital)
		return (_vm->_imuseDigital->getSoundStatus(sound) != 0);

	if (sound == _currentCDSound)
		return pollCD();

	if (_vm->_features & GF_HUMONGOUS) {
		if (sound == -2 || sound == 10001) {
			return !isSfxFinished();
		} else if (sound == -1 || sound == 10000 || sound == _currentMusic) {
			// getSoundStatus(), with a -1, will return the
			// ID number of the first active music it finds.
			if (_currentMusic)
				return (_musicChannelHandle.isActive()) ? _currentMusic : 0;
			else
				return _vm->_imuse->getSoundStatus(sound);
		}
	}

	if (isSoundInQueue(sound))
		return 1;

	if (!_vm->isResourceLoaded(rtSound, sound))
		return 0;

	if (_vm->_musicEngine)
		return _vm->_musicEngine->getSoundStatus(sound);

	return 0;
}

/**
 * Check whether the sound resource with the specified ID is still
 * used. This is invoked by ScummEngine::isResourceInUse, to determine
 * which resources can be expired from memory.
 * Technically, this works very similar to isSoundRunning, however it
 * calls IMuse::get_sound_active() instead of IMuse::getSoundStatus().
 * The difference between those two is in how they treat sounds which
 * are being faded out: get_sound_active() returns true even when the
 * sound is being faded out, while getSoundStatus() returns false in
 * that case.
 */
bool Sound::isSoundInUse(int sound) const {

	if (_vm->_imuseDigital)
		return (_vm->_imuseDigital->getSoundStatus(sound) != 0);

	if (sound == _currentCDSound)
		return pollCD() != 0;

	if (isSoundInQueue(sound))
		return true;

	if (!_vm->isResourceLoaded(rtSound, sound))
		return false;

	if (_vm->_imuse)
		return _vm->_imuse->get_sound_active(sound);

	return false;
}

bool Sound::isSoundInQueue(int sound) const {
	int i, num;

	i = _soundQue2Pos;
	while (i--) {
		if (_soundQue2Sound[i] == sound)
			return true;
	}

	i = 0;
	while (i < _soundQuePos) {
		num = _soundQue[i++];

		if (num > 0) {
			if (_soundQue[i + 0] == 0x10F && _soundQue[i + 1] == 8 && _soundQue[i + 2] == sound)
				return true;
			i += num;
		}
	}
	return false;
}

void Sound::stopSound(int a) {
	int i;

	if (_vm->_features & GF_HUMONGOUS) {
		if (a == -2 || a == 10001) {
			// Stop current sfx
		} else if (a == -1 || a == 10000) {
			// Stop current music
			if (_currentMusic)
				_vm->_mixer->stopID(_currentMusic);
			else
				_vm->_imuse->stopSound(_vm->_imuse->getSoundStatus(-1));
		}
	}

	if (a != 0 && a == _currentCDSound) {
		_currentCDSound = 0;
		stopCD();
		stopCDTimer();
	}

	if (!(_vm->_features & GF_DIGI_IMUSE))
		_vm->_mixer->stopID(a);

	if (_vm->_musicEngine)
		_vm->_musicEngine->stopSound(a);

	for (i = 0; i < ARRAYSIZE(_soundQue2Sound); i++) {
		if (_soundQue2Sound[i] == a) {
			_soundQue2Sound[i] = 0;
			_soundQue2Offset[i] = 0;
		}
	}
}

void Sound::stopAllSounds() {
	if (_currentCDSound != 0) {
		_currentCDSound = 0;
		stopCD();
		stopCDTimer();
	}

	// Clear the (secondary) sound queue
	_soundQue2Pos = 0;
	memset(_soundQue2Sound, 0, sizeof(_soundQue2Sound));
	memset(_soundQue2Offset, 0, sizeof(_soundQue2Offset));

	if (_vm->_musicEngine) {
		_vm->_musicEngine->stopAllSounds();
	}
	if (_vm->_imuse) {
		// FIXME: Maybe we could merge this call to clear_queue()
		// into IMuse::stopAllSounds() ?
		_vm->_imuse->clear_queue();
	}

	// Stop all SFX
	if (!_vm->_imuseDigital) {
		_vm->_mixer->stopAll();
	}
}

void Sound::soundKludge(int *list, int num) {
	int i;

	if (_vm->_imuseDigital) {
		_vm->_imuseDigital->parseScriptCmds(list[0], list[1], list[2], list[3], list[4],
												list[5], list[6], list[7]);
		return;
	}

	if (list[0] == -1) {
		processSoundQues();
	} else {
		_soundQue[_soundQuePos++] = num;
		
		for (i = 0; i < num; i++) {
			_soundQue[_soundQuePos++] = list[i];
		}
	}
}

void Sound::talkSound(uint32 a, uint32 b, int mode, int frame) {
	if (mode == 1) {
		_talk_sound_a1 = a;
		_talk_sound_b1 = b;
	} else {
		_talk_sound_a2 = a;
		_talk_sound_b2 = b;
	}

	_talk_sound_mode |= mode;
}

/* The sound code currently only supports General Midi.
 * General Midi is used in Day Of The Tentacle.
 * Roland music is also playable, but doesn't sound well.
 * A mapping between roland instruments and GM instruments
 * is needed.
 */

void Sound::setupSound() {
	delete _sfxFile;
	_sfxFile = openSfxFile();

	if (_vm->_gameId == GID_FT)
		_vm->VAR(_vm->VAR_VOICE_BUNDLE_LOADED) = _sfxFile->isOpen();
}

void Sound::pauseSounds(bool pause) {
	if (_vm->_imuse)
		_vm->_imuse->pause(pause);

	// Don't pause sounds if the game isn't active
	// FIXME - this is quite a nasty hack, replace with something cleaner, and w/o
	// having to access member vars directly!
	if (!_vm->_roomResource)
		return;

	_soundsPaused = pause;

	if (_vm->_imuseDigital) {
		_vm->_imuseDigital->pause(pause);
	}

	_vm->_mixer->pauseAll(pause);

	if ((_vm->_features & GF_AUDIOTRACKS) && _vm->VAR(_vm->VAR_MUSIC_TIMER) > 0) {
		if (pause)
			stopCDTimer();
		else
			startCDTimer();
	}
}

void Sound::startSfxSound(File *file, int file_size, PlayingSoundHandle *handle, int id) {

	AudioStream *input = NULL;
	
	switch (_soundMode) {
	case kMP3Mode:
#ifdef USE_MAD
		assert(file_size > 0);
		input = makeMP3Stream(file, file_size);
#endif
		break;
	case kVorbisMode:
#ifdef USE_VORBIS
		assert(file_size > 0);
		input = makeVorbisStream(file, file_size);
#endif
		break;
	case kFlacMode:
#ifdef USE_FLAC
		assert(file_size > 0);
		input = makeFlacStream(file, file_size);
#endif
		break;
	default:
		input = makeVOCStream(_sfxFile);
	}
	
	if (!input) {
		warning("startSfxSound failed to load sound");
		return;
	}

	if (_vm->_imuseDigital) {
		//_vm->_imuseDigital->stopSound(kTalkSoundID);
		_vm->_imuseDigital->startVoice(kTalkSoundID, input);
	} else {
		_vm->_mixer->playInputStream(handle, input, false, 255, 0, id);
	}
}

File *Sound::openSfxFile() {
	char buf[256];
	XORFile *file = new XORFile();
	_offsetTable = NULL;
	
	struct SoundFileExtensions {
		const char *ext;
		SoundMode mode;
	};
	
	const SoundFileExtensions extensions[] = {
#ifdef USE_FLAC
		{ "sof", kFlacMode },
#endif
#ifdef USE_MAD
		{ "so3", kMP3Mode },
#endif
#ifdef USE_VORBIS
		{ "sog", kVorbisMode },
#endif
		{ "sou", kVOCMode },
		{ 0, kVOCMode }
	};

	/* Try opening the file <_gameName>.sou first, eg tentacle.sou.
	 * That way, you can keep .sou files for multiple games in the
	 * same directory */
	
	int i, j;
	const char *basename[3] = { 0, 0, 0 };
	basename[0] = _vm->getGameName();
	basename[1] = "monster";
	
	for (j = 0; basename[j] && !file->isOpen(); ++j) {
		for (i = 0; extensions[i].ext; ++i) {
			sprintf(buf, "%s.%s", basename[j], extensions[i].ext);
			if (file->open(buf)) {
				_soundMode = extensions[i].mode;
				break;
			}
		}
	}

	if (!file->isOpen()) {
		if (_vm->_heversion >= 70)
			sprintf(buf, "%s.he2", _vm->getGameName());
		else
			sprintf(buf, "%s.tlk", _vm->getGameName());
		if (file->open(buf))
			file->setEnc(0x69);
		_soundMode = kVOCMode;
	} else if (_soundMode != kVOCMode) {
		/* Now load the 'offset' index in memory to be able to find the MP3 data

		   The format of the .SO3 file is easy :
		   - number of bytes of the 'index' part
		   - N times the following fields (4 bytes each) :
		   + offset in the original sound file
		   + offset of the MP3 data in the .SO3 file WITHOUT taking into account
		   the index field and the 'size' field
		   + the number of 'tags'
		   + the size of the MP3 data
		   - and then N times :
		   + the tags
		   + the MP3 data
		 */
		int size, compressed_offset;
		MP3OffsetTable *cur;
		compressed_offset = file->readUint32BE();
		_offsetTable = (MP3OffsetTable *) malloc(compressed_offset);
		_numSoundEffects = compressed_offset / 16;

		size = compressed_offset;
		cur = _offsetTable;
		while (size > 0) {
			cur->org_offset = file->readUint32BE();
			cur->new_offset = file->readUint32BE() + compressed_offset + 4; /* The + 4 is to take into accound the 'size' field */
			cur->num_tags = file->readUint32BE();
			cur->compressed_size = file->readUint32BE();
			size -= 4 * 4;
			cur++;
		}
	}

	return file;
}

bool Sound::isSfxFinished() const {
	return !_vm->_mixer->hasActiveSFXChannel();
}

// We use a real timer in an attempt to get better sync with CD tracks. This is
// necessary for games like Loom CD.

static void cd_timer_handler(void *refCon) {
	ScummEngine *scumm = (ScummEngine *)refCon;

	// FIXME: Turn off the timer when it's no longer needed. In theory, it
	// should be possible to check with pollCD(), but since CD sound isn't
	// properly restarted when reloading a saved game, I don't dare to.

	scumm->VAR(scumm->VAR_MUSIC_TIMER) += 6;
}

void Sound::startCDTimer() {
	int timer_interval;

	// The timer interval has been tuned for Loom CD and the Monkey 1
	// intro. I have to use 100 for Loom, or there will be a nasty stutter
	// when Chaos first appears, and I have to use 101 for Monkey 1 or the
	// intro music will be cut short.

	if (_vm->_gameId == GID_LOOM256)
		timer_interval = 100;
	else 
		timer_interval = 101;

	_vm->_timer->removeTimerProc(&cd_timer_handler);
	_vm->_timer->installTimerProc(&cd_timer_handler, 1000 * timer_interval, _vm);
}

void Sound::stopCDTimer() {
	_vm->_timer->removeTimerProc(&cd_timer_handler);
}

void Sound::playCDTrack(int track, int numLoops, int startFrame, int duration) {
	// Reset the music timer variable at the start of a new track
	_vm->VAR(_vm->VAR_MUSIC_TIMER) = 0;

	// Play it
	if (!_soundsPaused)
		AudioCD.play(track, numLoops, startFrame, duration);

	// Start the timer after starting the track. Starting an MP3 track is
	// almost instantaneous, but a CD player may take some time. Hopefully
	// play_cdrom() will block during that delay.
	startCDTimer();
}

void Sound::stopCD() {
	AudioCD.stop();
}

int Sound::pollCD() const {
	return AudioCD.isPlaying();
}

void Sound::updateCD() {
	AudioCD.updateCD();
}

} // End of namespace Scumm
