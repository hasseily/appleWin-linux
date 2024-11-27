#pragma once

class IDirectSoundBuffer;

struct _DSBUFFERDESC;
typedef const _DSBUFFERDESC* LPCDSBUFFERDESC;

// Sound
IDirectSoundBuffer * iCreateDirectSoundBuffer(LPCDSBUFFERDESC lpcDSBufferDesc);
