/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxKeyboard.c
 * @date 26/06/2010
 * @author iarwain@orx-project.org
 *
 * Android keyboard plugin implementation
 *
 */


#include <android/keycodes.h>

#include "orxPluginAPI.h"
#include "main/android/orxAndroid.h"
#include "main/android/orxAndroidActivity.h"

/** Module flags
 */
#define orxKEYBOARD_KU32_STATIC_FLAG_NONE       0x00000000 /**< No flags */

#define orxKEYBOARD_KU32_STATIC_FLAG_READY      0x00000001 /**< Ready flag */

#define orxKEYBOARD_KU32_STATIC_MASK_ALL        0xFFFFFFFF /**< All mask */

#define orxKEYBOARD_KU32_BOUND_KEY_NONE         0x00000000  /** No flags */

#define orxKEYBOARD_KU32_BOUND_KEY_VOLUME_UP    0x00000001  /**< Volume up flag */
#define orxKEYBOARD_KU32_BOUND_KEY_VOLUME_DOWN  0x00000002  /**< Volume down flag */

/** Misc
 */
#define orxKEYBOARD_KU32_BUFFER_SIZE            64
#define orxKEYBOARD_KU32_STRING_BUFFER_SIZE     (orxKEYBOARD_KU32_BUFFER_SIZE * 4 + 1)

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxKEYBOARD_STATIC_t
{
  orxU32            u32Flags;
  orxBOOL           abKeyPressed[orxKEYBOARD_KEY_NUMBER];
  orxU32            u32BoundKeys;

  orxU32            u32KeyReadIndex, u32KeyWriteIndex;
  orxU32            au32KeyBuffer[orxKEYBOARD_KU32_BUFFER_SIZE];
  orxCHAR           acStringBuffer[orxKEYBOARD_KU32_STRING_BUFFER_SIZE];
} orxKEYBOARD_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxKEYBOARD_STATIC sstKeyboard;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static void GameTextInputGetStateCB(void *ctx, const struct GameTextInputState *state)
{
  if(!state)
  {
    return;
  }

  orxString_NCopy(sstKeyboard.acStringBuffer, state->text_UTF8, sizeof(sstKeyboard.acStringBuffer) - 1);
  sstKeyboard.acStringBuffer[sizeof(sstKeyboard.acStringBuffer) - 1] = orxNULL;
}

static void orxKeyboard_Android_ClearKeyboardBuffer()
{
  GameTextInputState state;
  orxMemory_Zero(&state, sizeof(GameTextInputState));
  GameActivity_setTextInputState(orxAndroid_GetGameActivity(), &state);
}

static void orxKeyboard_Android_ConsumeKeyboardBuffer()
{
  GameActivity_getTextInputState(
    orxAndroid_GetGameActivity(),
    GameTextInputGetStateCB,
    NULL);

  orxKeyboard_Android_ClearKeyboardBuffer();
}

static orxKEYBOARD_KEY orxFASTCALL orxKeyboard_Android_GetKey(orxU32 _eKey)
{
  orxKEYBOARD_KEY eResult;

  /* Depending on key */
  switch(_eKey)
  {
    case AKEYCODE_BACK:               { eResult = orxKEYBOARD_KEY_ESCAPE; break; }
    case AKEYCODE_SPACE:              { eResult = orxKEYBOARD_KEY_SPACE; break; }
    case AKEYCODE_ENTER:              { eResult = orxKEYBOARD_KEY_ENTER; break; }
    case AKEYCODE_BUTTON_SELECT:      { eResult = orxKEYBOARD_KEY_ENTER; break; }
    case AKEYCODE_DEL:                { eResult = orxKEYBOARD_KEY_BACKSPACE; break; }
    case AKEYCODE_TAB:                { eResult = orxKEYBOARD_KEY_TAB; break; }
    case AKEYCODE_PAGE_UP:            { eResult = orxKEYBOARD_KEY_PAGE_UP; break; }
    case AKEYCODE_PAGE_DOWN:          { eResult = orxKEYBOARD_KEY_PAGE_DOWN; break; }
    case AKEYCODE_PLUS:               { eResult = orxKEYBOARD_KEY_NUMPAD_ADD; break; }
    case AKEYCODE_STAR:               { eResult = orxKEYBOARD_KEY_NUMPAD_MULTIPLY; break; }
    case AKEYCODE_MEDIA_PLAY_PAUSE:   { eResult = orxKEYBOARD_KEY_PAUSE; break; }
    case AKEYCODE_ALT_RIGHT:          { eResult = orxKEYBOARD_KEY_RALT; break; }
    case AKEYCODE_SHIFT_RIGHT:        { eResult = orxKEYBOARD_KEY_RSHIFT; break; }
    case AKEYCODE_ALT_LEFT:           { eResult = orxKEYBOARD_KEY_LALT; break; }
    case AKEYCODE_SHIFT_LEFT:         { eResult = orxKEYBOARD_KEY_LSHIFT; break; }
    case AKEYCODE_MENU:               { eResult = orxKEYBOARD_KEY_MENU; break; }
    case AKEYCODE_LEFT_BRACKET:       { eResult = orxKEYBOARD_KEY_LBRACKET; break; }
    case AKEYCODE_RIGHT_BRACKET:      { eResult = orxKEYBOARD_KEY_RBRACKET; break; }
    case AKEYCODE_SEMICOLON:          { eResult = orxKEYBOARD_KEY_SEMICOLON; break; }
    case AKEYCODE_COMMA:              { eResult = orxKEYBOARD_KEY_COMMA; break; }
    case AKEYCODE_PERIOD:             { eResult = orxKEYBOARD_KEY_PERIOD; break; }
    case AKEYCODE_APOSTROPHE:         { eResult = orxKEYBOARD_KEY_QUOTE; break; }
    case AKEYCODE_SLASH:              { eResult = orxKEYBOARD_KEY_SLASH; break; }
    case AKEYCODE_BACKSLASH:          { eResult = orxKEYBOARD_KEY_BACKSLASH; break; }
    case AKEYCODE_EQUALS:             { eResult = orxKEYBOARD_KEY_EQUAL; break; }
    case AKEYCODE_MINUS:              { eResult = orxKEYBOARD_KEY_DASH; break; }
    case AKEYCODE_DPAD_UP:            { eResult = orxKEYBOARD_KEY_UP; break; }
    case AKEYCODE_DPAD_RIGHT:         { eResult = orxKEYBOARD_KEY_RIGHT; break; }
    case AKEYCODE_DPAD_DOWN:          { eResult = orxKEYBOARD_KEY_DOWN; break; }
    case AKEYCODE_DPAD_LEFT:          { eResult = orxKEYBOARD_KEY_LEFT; break; }
    case AKEYCODE_VOLUME_DOWN:        { eResult = orxKEYBOARD_KEY_VOLUME_DOWN; break; }
    case AKEYCODE_VOLUME_UP:          { eResult = orxKEYBOARD_KEY_VOLUME_UP; break; }
    case AKEYCODE_A:                  { eResult = orxKEYBOARD_KEY_A; break; }
    case AKEYCODE_Z:                  { eResult = orxKEYBOARD_KEY_Z; break; }
    case AKEYCODE_E:                  { eResult = orxKEYBOARD_KEY_E; break; }
    case AKEYCODE_R:                  { eResult = orxKEYBOARD_KEY_R; break; }
    case AKEYCODE_T:                  { eResult = orxKEYBOARD_KEY_T; break; }
    case AKEYCODE_Y:                  { eResult = orxKEYBOARD_KEY_Y; break; }
    case AKEYCODE_U:                  { eResult = orxKEYBOARD_KEY_U; break; }
    case AKEYCODE_I:                  { eResult = orxKEYBOARD_KEY_I; break; }
    case AKEYCODE_O:                  { eResult = orxKEYBOARD_KEY_O; break; }
    case AKEYCODE_P:                  { eResult = orxKEYBOARD_KEY_P; break; }
    case AKEYCODE_Q:                  { eResult = orxKEYBOARD_KEY_Q; break; }
    case AKEYCODE_S:                  { eResult = orxKEYBOARD_KEY_S; break; }
    case AKEYCODE_D:                  { eResult = orxKEYBOARD_KEY_D; break; }
    case AKEYCODE_F:                  { eResult = orxKEYBOARD_KEY_F; break; }
    case AKEYCODE_G:                  { eResult = orxKEYBOARD_KEY_G; break; }
    case AKEYCODE_H:                  { eResult = orxKEYBOARD_KEY_H; break; }
    case AKEYCODE_J:                  { eResult = orxKEYBOARD_KEY_J; break; }
    case AKEYCODE_K:                  { eResult = orxKEYBOARD_KEY_K; break; }
    case AKEYCODE_L:                  { eResult = orxKEYBOARD_KEY_L; break; }
    case AKEYCODE_M:                  { eResult = orxKEYBOARD_KEY_M; break; }
    case AKEYCODE_W:                  { eResult = orxKEYBOARD_KEY_W; break; }
    case AKEYCODE_X:                  { eResult = orxKEYBOARD_KEY_X; break; }
    case AKEYCODE_C:                  { eResult = orxKEYBOARD_KEY_C; break; }
    case AKEYCODE_V:                  { eResult = orxKEYBOARD_KEY_V; break; }
    case AKEYCODE_B:                  { eResult = orxKEYBOARD_KEY_B; break; }
    case AKEYCODE_N:                  { eResult = orxKEYBOARD_KEY_N; break; }
    case AKEYCODE_0:                  { eResult = orxKEYBOARD_KEY_0; break; }
    case AKEYCODE_1:                  { eResult = orxKEYBOARD_KEY_1; break; }
    case AKEYCODE_2:                  { eResult = orxKEYBOARD_KEY_2; break; }
    case AKEYCODE_3:                  { eResult = orxKEYBOARD_KEY_3; break; }
    case AKEYCODE_4:                  { eResult = orxKEYBOARD_KEY_4; break; }
    case AKEYCODE_5:                  { eResult = orxKEYBOARD_KEY_5; break; }
    case AKEYCODE_6:                  { eResult = orxKEYBOARD_KEY_6; break; }
    case AKEYCODE_7:                  { eResult = orxKEYBOARD_KEY_7; break; }
    case AKEYCODE_8:                  { eResult = orxKEYBOARD_KEY_8; break; }
    case AKEYCODE_9:                  { eResult = orxKEYBOARD_KEY_9; break; }
    default:                          { eResult = orxKEYBOARD_KEY_NONE; break; }
  }

  /* Done! */
  return eResult;
}

static void orxFASTCALL orxKeyboard_Android_UpdateKeyFilterState(const orxCLOCK_INFO* pClockInfo, void* pContext) {
  orxU32 u32BoundKeys;
  const orxSTRING zInputSet = orxNULL;
  const orxSTRING zName;

#define orxKEYBOARD_UPDATE_BOUND_KEY(KEY)                                                                                                       \
  do                                                                                                                                            \
  {                                                                                                                                             \
    if(orxInput_GetBoundInput(orxINPUT_TYPE_KEYBOARD_KEY, orxKEYBOARD_KEY_##KEY, orxINPUT_MODE_FULL, 0, &zName, orxNULL) != orxSTATUS_FAILURE)  \
    {                                                                                                                                           \
      orxFLAG_SET(u32BoundKeys, orxKEYBOARD_KU32_BOUND_KEY_##KEY, orxKEYBOARD_KU32_BOUND_KEY_NONE);                                             \
    }                                                                                                                                           \
  } while(orxFALSE)

  /* Unbound keys are handled by system */
  u32BoundKeys = orxKEYBOARD_KU32_BOUND_KEY_NONE;

  while((zInputSet = orxInput_GetNextSet(zInputSet)))
  {
    if(orxInput_IsSetEnabled(zInputSet))
    {
      /* Pushes set */
      orxInput_PushSet(zInputSet);

      orxKEYBOARD_UPDATE_BOUND_KEY(VOLUME_UP);
      orxKEYBOARD_UPDATE_BOUND_KEY(VOLUME_DOWN);

      /* Pops previous set */
      orxInput_PopSet();
    }
  }

  /* Updates bound keys */
  sstKeyboard.u32BoundKeys = u32BoundKeys;

#undef orxKEYBOARD_UPDATE_BOUND_KEY
}

static bool orxKeyboard_Android_KeyEventFilter(const GameActivityKeyEvent *event)
{
  bool bAllowKey;

  switch(event->keyCode)
  {
    case AKEYCODE_VOLUME_DOWN:
    {
      bAllowKey = orxFLAG_TEST(sstKeyboard.u32BoundKeys, orxKEYBOARD_KU32_BOUND_KEY_VOLUME_DOWN);
      break;
    }
    case AKEYCODE_VOLUME_UP:
    {
      bAllowKey = orxFLAG_TEST(sstKeyboard.u32BoundKeys, orxKEYBOARD_KU32_BOUND_KEY_VOLUME_UP);
      break;
    }
    case AKEYCODE_VOLUME_MUTE:
    case AKEYCODE_CAMERA:
    case AKEYCODE_ZOOM_IN:
    case AKEYCODE_ZOOM_OUT:
    {
      /* Note : Copied from android_native_app_glue.c in AGDK */
      bAllowKey = false;
      break;
    }
    default:
    {
      bAllowKey = true;
      break;
    }
  }

  return bAllowKey;
}

static orxSTATUS orxFASTCALL orxKeyboard_Android_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxANDROID_EVENT_PAYLOAD *pstPayload;
  orxKEYBOARD_KEY eKey;

  /* Gets payload */
  pstPayload = (orxANDROID_EVENT_PAYLOAD *)_pstEvent->pstPayload;

  /* Depending on ID */
  switch(_pstEvent->eID)
  {
    case orxANDROID_EVENT_KEY_DOWN:
    {
      eKey = orxKeyboard_Android_GetKey(pstPayload->stKey.u32KeyCode);
      if(eKey != orxKEYBOARD_KEY_NONE)
      {
        sstKeyboard.abKeyPressed[eKey] = orxTRUE;

        /* Stores it */
        sstKeyboard.au32KeyBuffer[sstKeyboard.u32KeyWriteIndex] = pstPayload->stKey.u32KeyCode;
        sstKeyboard.u32KeyWriteIndex =
                (sstKeyboard.u32KeyWriteIndex + 1) & (orxKEYBOARD_KU32_BUFFER_SIZE - 1);

        /* Full? */
        if(sstKeyboard.u32KeyReadIndex == sstKeyboard.u32KeyWriteIndex)
        {
          /* Bounces read index */
          sstKeyboard.u32KeyReadIndex =
                  (sstKeyboard.u32KeyReadIndex + 1) & (orxKEYBOARD_KU32_BUFFER_SIZE - 1);
        }
      }
      break;
    }
    case orxANDROID_EVENT_KEY_UP:
    {
      eKey = orxKeyboard_Android_GetKey(pstPayload->stKey.u32KeyCode);
      if(eKey != orxKEYBOARD_KEY_NONE)
      {
        sstKeyboard.abKeyPressed[eKey] = orxFALSE;
      }
      break;
    }
    default:
    {
      break;
    }
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxKeyboard_Android_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Wasn't already initialized? */
  if(!(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));

    /* Adds our keyboard event handlers */
    if((eResult = orxEvent_AddHandler(orxEVENT_TYPE_ANDROID, orxKeyboard_Android_EventHandler)) != orxSTATUS_FAILURE)
    {
      /* Filters events */
      orxEvent_SetHandlerIDFlags(orxKeyboard_Android_EventHandler, orxEVENT_TYPE_ANDROID, orxNULL, orxEVENT_GET_FLAG(orxANDROID_EVENT_KEY_DOWN) | orxEVENT_GET_FLAG(orxANDROID_EVENT_KEY_UP), orxEVENT_KU32_MASK_ID_ALL);

      /* Updates status */
      sstKeyboard.u32Flags |= orxKEYBOARD_KU32_STATIC_FLAG_READY;
    }

    /* Inits key filter */
    orxAndroid_SetKeyFilter(orxKeyboard_Android_KeyEventFilter);

    /* Registers filter state function */
    orxClock_Register(orxClock_Get(orxCLOCK_KZ_CORE), orxKeyboard_Android_UpdateKeyFilterState, orxNULL, orxMODULE_ID_KEYBOARD, orxCLOCK_PRIORITY_NORMAL);
  }

  /* Done! */
  return eResult;
}

extern "C" void orxFASTCALL orxKeyboard_Android_Exit()
{
  /* Was initialized? */
  if(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_ANDROID, orxKeyboard_Android_EventHandler);

    /* Clears key filter */
    orxAndroid_SetKeyFilter(NULL);

    /* Unregisters filter state function */
    orxClock_Unregister(orxClock_Get(orxCLOCK_KZ_CORE), orxKeyboard_Android_UpdateKeyFilterState);

    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));
  }

  return;
}

extern "C" orxBOOL orxFASTCALL orxKeyboard_Android_IsKeyPressed(orxKEYBOARD_KEY _eKey)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(_eKey < orxKEYBOARD_KEY_NUMBER);
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = sstKeyboard.abKeyPressed[_eKey];

  /* Done! */
  return bResult;
}

extern "C" const orxSTRING orxFASTCALL orxKeyboard_Android_GetKeyDisplayName(orxKEYBOARD_KEY _eKey)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(_eKey < orxKEYBOARD_KEY_NUMBER);
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Gets key name */
  zResult = orxKeyboard_GetKeyName(_eKey);

  /* Success? */
  if(zResult != orxSTRING_EMPTY)
  {
    /* Skips prefix */
    zResult += 4;
  }

  /* Done! */
  return zResult;
}

extern "C" orxKEYBOARD_KEY orxFASTCALL orxKeyboard_Android_ReadKey()
{
  orxKEYBOARD_KEY eResult;

  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Not empty? */
  if(sstKeyboard.u32KeyReadIndex != sstKeyboard.u32KeyWriteIndex)
  {
    /* Updates result */
    eResult = orxKeyboard_Android_GetKey((orxS32)sstKeyboard.au32KeyBuffer[sstKeyboard.u32KeyReadIndex]);

    /* Updates read index */
    sstKeyboard.u32KeyReadIndex = (sstKeyboard.u32KeyReadIndex + 1) & (orxKEYBOARD_KU32_BUFFER_SIZE - 1);
  }
  else
  {
    /* Updates result */
    eResult = orxKEYBOARD_KEY_NONE;
  }

  /* Done! */
  return eResult;
}

extern "C" const orxSTRING orxFASTCALL orxKeyboard_Android_ReadString()
{
  const orxSTRING zResult = sstKeyboard.acStringBuffer;

  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  orxKeyboard_Android_ConsumeKeyboardBuffer();

  /* Done! */
  return zResult;
}

extern "C" void orxFASTCALL orxKeyboard_Android_ClearBuffer()
{
  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Clears all buffer indices */
  sstKeyboard.u32KeyReadIndex   = 0;
  sstKeyboard.u32KeyWriteIndex  = 0;

  /* Clears keyboard buffers */
  orxMemory_Zero(sstKeyboard.acStringBuffer, sizeof(sstKeyboard.acStringBuffer));
  orxKeyboard_Android_ClearKeyboardBuffer();

  /* Done! */
  return;
}

extern "C" orxSTATUS orxFASTCALL orxKeyboard_Android_Show(orxBOOL _bShow)
{
  if(_bShow)
  {
    GameActivity_showSoftInput(orxAndroid_GetGameActivity(), 0);
  }
  else
  {
    GameActivity_hideSoftInput(orxAndroid_GetGameActivity(), 0);
  }

  /* Done */
  return orxSTATUS_SUCCESS;
}

/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(KEYBOARD);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_Init, KEYBOARD, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_Exit, KEYBOARD, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_IsKeyPressed, KEYBOARD, IS_KEY_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_GetKeyDisplayName, KEYBOARD, GET_KEY_DISPLAY_NAME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_ReadKey, KEYBOARD, READ_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_ReadString, KEYBOARD, READ_STRING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_ClearBuffer, KEYBOARD, CLEAR_BUFFER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_Show, KEYBOARD, SHOW);
orxPLUGIN_USER_CORE_FUNCTION_END();
