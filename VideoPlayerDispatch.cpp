#include "VideoPlayerDispatch.h"

HRESULT VideoPlayerDispatch::Invoke(
    DISPID  dispIdMember,
    REFIID  /*riid*/,
    LCID  /*lcid*/,
    WORD  /*wFlags*/,
    DISPPARAMS FAR* pDispParams,
    VARIANT FAR*  /*pVarResult*/,
    EXCEPINFO FAR*  /*pExcepInfo*/,
    unsigned int FAR*  /*puArgErr*/)
{
    if (!pDispParams)
        return E_POINTER;

    if (pDispParams->cNamedArgs != 0)
        return DISP_E_NONAMEDARGS;

    HRESULT hr = DISP_E_MEMBERNOTFOUND;

    switch (dispIdMember)
    {
    case DISPID_WMPCOREEVENT_OPENSTATECHANGE:
        OpenStateChange(pDispParams->rgvarg[0].lVal /* NewState */);
        break;

    case DISPID_WMPCOREEVENT_PLAYSTATECHANGE:
        PlayStateChange(pDispParams->rgvarg[0].lVal /* NewState */);
        break;

    case DISPID_WMPCOREEVENT_AUDIOLANGUAGECHANGE:
        AudioLanguageChange(pDispParams->rgvarg[0].lVal /* LangID */);
        break;

    case DISPID_WMPCOREEVENT_STATUSCHANGE:
        StatusChange();
        break;

    case DISPID_WMPCOREEVENT_SCRIPTCOMMAND:
        ScriptCommand(pDispParams->rgvarg[1].bstrVal /* scType */, pDispParams->rgvarg[0].bstrVal /* Param */);
        break;

    case DISPID_WMPCOREEVENT_NEWSTREAM:
        NewStream();
        break;

    case DISPID_WMPCOREEVENT_DISCONNECT:
        Disconnect(pDispParams->rgvarg[0].lVal /* Result */);
        break;

    case DISPID_WMPCOREEVENT_BUFFERING:
        Buffering(pDispParams->rgvarg[0].boolVal /* Start */);
        break;

    case DISPID_WMPCOREEVENT_ERROR:
        Error();
        break;

    case DISPID_WMPCOREEVENT_WARNING:
        Warning(pDispParams->rgvarg[1].lVal /* WarningType */, pDispParams->rgvarg[0].lVal /* Param */, pDispParams->rgvarg[2].bstrVal /* Description */);
        break;

    case DISPID_WMPCOREEVENT_ENDOFSTREAM:
        EndOfStream(pDispParams->rgvarg[0].lVal /* Result */);
        break;

    case DISPID_WMPCOREEVENT_POSITIONCHANGE:
        PositionChange(pDispParams->rgvarg[1].dblVal /* oldPosition */, pDispParams->rgvarg[0].dblVal /* newPosition */);
        break;

    case DISPID_WMPCOREEVENT_MARKERHIT:
        MarkerHit(pDispParams->rgvarg[0].lVal /* MarkerNum */);
        break;

    case DISPID_WMPCOREEVENT_DURATIONUNITCHANGE:
        DurationUnitChange(pDispParams->rgvarg[0].lVal /* NewDurationUnit */);
        break;

    case DISPID_WMPCOREEVENT_CDROMMEDIACHANGE:
        CdromMediaChange(pDispParams->rgvarg[0].lVal /* CdromNum */);
        break;

    case DISPID_WMPCOREEVENT_PLAYLISTCHANGE:
        PlaylistChange(pDispParams->rgvarg[1].pdispVal /* Playlist */, (WMPPlaylistChangeEventType)pDispParams->rgvarg[0].lVal /* change */);
        break;

    case DISPID_WMPCOREEVENT_CURRENTPLAYLISTCHANGE:
        CurrentPlaylistChange((WMPPlaylistChangeEventType)pDispParams->rgvarg[0].lVal /* change */);
        break;

    case DISPID_WMPCOREEVENT_CURRENTPLAYLISTITEMAVAILABLE:
        CurrentPlaylistItemAvailable(pDispParams->rgvarg[0].bstrVal /*  bstrItemName */);
        break;

    case DISPID_WMPCOREEVENT_MEDIACHANGE:
        MediaChange(pDispParams->rgvarg[0].pdispVal /* Item */);
        break;

    case DISPID_WMPCOREEVENT_CURRENTMEDIAITEMAVAILABLE:
        CurrentMediaItemAvailable(pDispParams->rgvarg[0].bstrVal /* bstrItemName */);
        break;

    case DISPID_WMPCOREEVENT_CURRENTITEMCHANGE:
        CurrentItemChange(pDispParams->rgvarg[0].pdispVal /* pdispMedia */);
        break;

    case DISPID_WMPCOREEVENT_MEDIACOLLECTIONCHANGE:
        MediaCollectionChange();
        break;

    case DISPID_WMPCOREEVENT_MEDIACOLLECTIONATTRIBUTESTRINGADDED:
        MediaCollectionAttributeStringAdded(pDispParams->rgvarg[1].bstrVal /* bstrAttribName */, pDispParams->rgvarg[0].bstrVal /* bstrAttribVal */);
        break;

    case DISPID_WMPCOREEVENT_MEDIACOLLECTIONATTRIBUTESTRINGREMOVED:
        MediaCollectionAttributeStringRemoved(pDispParams->rgvarg[1].bstrVal /* bstrAttribName */, pDispParams->rgvarg[0].bstrVal /* bstrAttribVal */);
        break;

    case DISPID_WMPCOREEVENT_MEDIACOLLECTIONATTRIBUTESTRINGCHANGED:
        MediaCollectionAttributeStringChanged(pDispParams->rgvarg[2].bstrVal /* bstrAttribName */, pDispParams->rgvarg[1].bstrVal /* bstrOldAttribVal */, pDispParams->rgvarg[0].bstrVal /* bstrNewAttribVal */);
        break;

    case DISPID_WMPCOREEVENT_PLAYLISTCOLLECTIONCHANGE:
        PlaylistCollectionChange();
        break;

    case DISPID_WMPCOREEVENT_PLAYLISTCOLLECTIONPLAYLISTADDED:
        PlaylistCollectionPlaylistAdded(pDispParams->rgvarg[0].bstrVal /* bstrPlaylistName */);
        break;

    case DISPID_WMPCOREEVENT_PLAYLISTCOLLECTIONPLAYLISTREMOVED:
        PlaylistCollectionPlaylistRemoved(pDispParams->rgvarg[0].bstrVal /* bstrPlaylistName */);
        break;

    case DISPID_WMPCOREEVENT_PLAYLISTCOLLECTIONPLAYLISTSETASDELETED:
        PlaylistCollectionPlaylistSetAsDeleted(pDispParams->rgvarg[1].bstrVal /* bstrPlaylistName */, pDispParams->rgvarg[0].boolVal /* varfIsDeleted */);
        break;

    case DISPID_WMPCOREEVENT_MODECHANGE:
        ModeChange(pDispParams->rgvarg[1].bstrVal /* ModeName */, pDispParams->rgvarg[0].boolVal /* NewValue */);
        break;

    case DISPID_WMPCOREEVENT_MEDIAERROR:
        MediaError(pDispParams->rgvarg[0].pdispVal /* pMediaObject */);
        break;

    case DISPID_WMPCOREEVENT_OPENPLAYLISTSWITCH:
        OpenPlaylistSwitch(pDispParams->rgvarg[0].pdispVal /* pItem */);
        break;

    case DISPID_WMPCOREEVENT_DOMAINCHANGE:
        DomainChange(pDispParams->rgvarg[0].bstrVal /* bstrDomain */);
        break;

    case DISPID_WMPOCXEVENT_SWITCHEDTOPLAYERAPPLICATION:
        SwitchedToPlayerApplication();
        break;

    case DISPID_WMPOCXEVENT_SWITCHEDTOCONTROL:
        SwitchedToControl();
        break;

    case DISPID_WMPOCXEVENT_PLAYERDOCKEDSTATECHANGE:
        PlayerDockedStateChange();
        break;

    case DISPID_WMPOCXEVENT_PLAYERRECONNECT:
        PlayerReconnect();
        break;

    case DISPID_WMPOCXEVENT_CLICK:
        Click(pDispParams->rgvarg[3].iVal /* nButton */, pDispParams->rgvarg[2].iVal /* nShiftState */, pDispParams->rgvarg[1].lVal /* fX */, pDispParams->rgvarg[0].lVal /* fY */);
        break;

    case DISPID_WMPOCXEVENT_DOUBLECLICK:
        DoubleClick(pDispParams->rgvarg[3].iVal /* nButton */, pDispParams->rgvarg[2].iVal /* nShiftState */, pDispParams->rgvarg[1].lVal /* fX */, pDispParams->rgvarg[0].lVal /* fY */);
        break;

    case DISPID_WMPOCXEVENT_KEYDOWN:
        KeyDown(pDispParams->rgvarg[1].iVal /* nKeyCode */, pDispParams->rgvarg[0].iVal /* nShiftState */);
        break;

    case DISPID_WMPOCXEVENT_KEYPRESS:
        KeyPress(pDispParams->rgvarg[0].iVal /* nKeyAscii */);
        break;

    case DISPID_WMPOCXEVENT_KEYUP:
        KeyUp(pDispParams->rgvarg[1].iVal /* nKeyCode */, pDispParams->rgvarg[0].iVal /* nShiftState */);
        break;

    case DISPID_WMPOCXEVENT_MOUSEDOWN:
        MouseDown(pDispParams->rgvarg[3].iVal /* nButton */, pDispParams->rgvarg[2].iVal /* nShiftState */, pDispParams->rgvarg[1].lVal /* fX */, pDispParams->rgvarg[0].lVal /* fY */);
        break;

    case DISPID_WMPOCXEVENT_MOUSEMOVE:
        MouseMove(pDispParams->rgvarg[3].iVal /* nButton */, pDispParams->rgvarg[2].iVal /* nShiftState */, pDispParams->rgvarg[1].lVal /* fX */, pDispParams->rgvarg[0].lVal /* fY */);
        break;

    case DISPID_WMPOCXEVENT_MOUSEUP:
        MouseUp(pDispParams->rgvarg[3].iVal /* nButton */, pDispParams->rgvarg[2].iVal /* nShiftState */, pDispParams->rgvarg[1].lVal /* fX */, pDispParams->rgvarg[0].lVal /* fY */);
        break;
    }

    return(hr);
}

// Sent when the control changes OpenState
void VideoPlayerDispatch::OpenStateChange(long /*NewState*/)
{
    return;
}

// Sent when the control changes PlayState
void VideoPlayerDispatch::PlayStateChange(long /*NewState*/)
{
    return;
}

// Sent when the audio language changes
void VideoPlayerDispatch::AudioLanguageChange(long /*LangID*/)
{
    return;
}

// Sent when the status string changes
void VideoPlayerDispatch::StatusChange()
{
    return;
}

// Sent when a synchronized command or URL is received
void VideoPlayerDispatch::ScriptCommand(BSTR /*scType*/, BSTR /*Param*/)
{
    return;
}

// Sent when a new stream is encountered (obsolete)
void VideoPlayerDispatch::NewStream()
{
    return;
}

// Sent when the control is disconnected from the server (obsolete)
void VideoPlayerDispatch::Disconnect(long /*Result*/)
{
    return;
}

// Sent when the control begins or ends buffering
void VideoPlayerDispatch::Buffering(VARIANT_BOOL /*Start*/)
{
    return;
}

// Sent when the control has an error condition
void VideoPlayerDispatch::Error()
{
    return;
}

// Sent when the control has an warning condition (obsolete)
void VideoPlayerDispatch::Warning(long /*WarningType*/, long /*Param*/, BSTR /*Description*/)
{
    return;
}

// Sent when the media has reached end of stream
void VideoPlayerDispatch::EndOfStream(long /*Result*/)
{
    return;
}

// Indicates that the current position of the movie has changed
void VideoPlayerDispatch::PositionChange(double /*oldPosition*/, double /*newPosition*/)
{
    return;
}

// Sent when a marker is reached
void VideoPlayerDispatch::MarkerHit(long /*MarkerNum*/)
{
    return;
}

// Indicates that the unit used to express duration and position has changed
void VideoPlayerDispatch::DurationUnitChange(long /*NewDurationUnit*/)
{
    return;
}

// Indicates that the CD ROM media has changed
void VideoPlayerDispatch::CdromMediaChange(long /*CdromNum*/)
{
    return;
}

// Sent when a playlist changes
void VideoPlayerDispatch::PlaylistChange(IDispatch* /*Playlist*/, WMPPlaylistChangeEventType /*change*/)
{
    return;
}

// Sent when the current playlist changes
void VideoPlayerDispatch::CurrentPlaylistChange(WMPPlaylistChangeEventType /*change*/)
{
    return;
}

// Sent when a current playlist item becomes available
void VideoPlayerDispatch::CurrentPlaylistItemAvailable(BSTR /*bstrItemName*/)
{
    return;
}

// Sent when a media object changes
void VideoPlayerDispatch::MediaChange(IDispatch* /*Item*/)
{
    return;
}

// Sent when a current media item becomes available
void VideoPlayerDispatch::CurrentMediaItemAvailable(BSTR /*bstrItemName*/)
{
    return;
}

// Sent when the item selection on the current playlist changes
void VideoPlayerDispatch::CurrentItemChange(IDispatch* /*pdispMedia*/)
{
    return;
}

// Sent when the media collection needs to be requeried
void VideoPlayerDispatch::MediaCollectionChange()
{
    return;
}

// Sent when an attribute string is added in the media collection
void VideoPlayerDispatch::MediaCollectionAttributeStringAdded(BSTR /*bstrAttribName*/, BSTR /*bstrAttribVal*/)
{
    return;
}

// Sent when an attribute string is removed from the media collection
void VideoPlayerDispatch::MediaCollectionAttributeStringRemoved(BSTR /*bstrAttribName*/, BSTR /*bstrAttribVal*/)
{
    return;
}

// Sent when an attribute string is changed in the media collection
void VideoPlayerDispatch::MediaCollectionAttributeStringChanged(BSTR /*bstrAttribName*/, BSTR /*bstrOldAttribVal*/, BSTR /*bstrNewAttribVal*/)
{
    return;
}

// Sent when playlist collection needs to be requeried
void VideoPlayerDispatch::PlaylistCollectionChange()
{
    return;
}

// Sent when a playlist is added to the playlist collection
void VideoPlayerDispatch::PlaylistCollectionPlaylistAdded(BSTR /*bstrPlaylistName*/)
{
    return;
}

// Sent when a playlist is removed from the playlist collection
void VideoPlayerDispatch::PlaylistCollectionPlaylistRemoved(BSTR /*bstrPlaylistName*/)
{
    return;
}

// Sent when a playlist has been set or reset as deleted
void VideoPlayerDispatch::PlaylistCollectionPlaylistSetAsDeleted(BSTR /*bstrPlaylistName*/, VARIANT_BOOL /*varfIsDeleted*/)
{
    return;
}

// Playlist playback mode has changed
void VideoPlayerDispatch::ModeChange(BSTR /*ModeName*/, VARIANT_BOOL /*NewValue*/)
{
    return;
}

// Sent when the media object has an error condition
void VideoPlayerDispatch::MediaError(IDispatch* /*pMediaObject*/)
{
    return;
}

// Current playlist switch with no open state change
void VideoPlayerDispatch::OpenPlaylistSwitch(IDispatch* /*pItem*/)
{
    return;
}

// Sent when the current DVD domain changes
void VideoPlayerDispatch::DomainChange(BSTR /*bstrDomain*/)
{
    return;
}

// Sent when display switches to player application
void VideoPlayerDispatch::SwitchedToPlayerApplication()
{
    return;
}

// Sent when display switches to control
void VideoPlayerDispatch::SwitchedToControl()
{
    return;
}

// Sent when the player docks or undocks
void VideoPlayerDispatch::PlayerDockedStateChange()
{
    return;
}

// Sent when the OCX reconnects to the player
void VideoPlayerDispatch::PlayerReconnect()
{
    return;
}

// Occurs when a user clicks the mouse
void VideoPlayerDispatch::Click(short /*nButton*/, short /*nShiftState*/, long /*fX*/, long /*fY*/)
{
    return;
}

// Occurs when a user double-clicks the mouse
void VideoPlayerDispatch::DoubleClick(short /*nButton*/, short /*nShiftState*/, long /*fX*/, long /*fY*/)
{
    return;
}

// Occurs when a key is pressed
void VideoPlayerDispatch::KeyDown(short /*nKeyCode*/, short /*nShiftState*/)
{
    return;
}

// Occurs when a key is pressed and released
void VideoPlayerDispatch::KeyPress(short /*nKeyAscii*/)
{
    return;
}

// Occurs when a key is released
void VideoPlayerDispatch::KeyUp(short /*nKeyCode*/, short /*nShiftState*/)
{
    return;
}

// Occurs when a mouse button is pressed
void VideoPlayerDispatch::MouseDown(short /*nButton*/, short /*nShiftState*/, long /*fX*/, long /*fY*/)
{
    return;
}

// Occurs when a mouse pointer is moved
void VideoPlayerDispatch::MouseMove(short /*nButton*/, short /*nShiftState*/, long /*fX*/, long /*fY*/)
{
    return;
}

// Occurs when a mouse button is released
void VideoPlayerDispatch::MouseUp(short /*nButton*/, short /*nShiftState*/, long /*fX*/, long /*fY*/)
{
    return;
}