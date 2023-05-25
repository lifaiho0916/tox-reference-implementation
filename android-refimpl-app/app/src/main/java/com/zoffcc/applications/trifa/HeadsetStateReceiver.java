package com.zoffcc.applications.trifa;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import static com.zoffcc.applications.trifa.CallingActivity.update_audio_device_icon;
import static com.zoffcc.applications.trifa.ConferenceAudioActivity.update_group_audio_device_icon;
import static com.zoffcc.applications.trifa.HelperGeneric.set_audio_to_ear;
import static com.zoffcc.applications.trifa.HelperGeneric.set_audio_to_headset;
import static com.zoffcc.applications.trifa.HelperGeneric.set_audio_to_loudspeaker;
import static com.zoffcc.applications.trifa.MainActivity.audio_manager_s;

class HeadsetStateReceiver extends BroadcastReceiver
{
    static final String TAG = "trifa.HeadsetStReceiver";

    @Override
    public void onReceive(Context context, Intent intent)
    {
        try
        {
            if (isInitialStickyBroadcast())
            {
                Log.i(TAG, "AUDIOROUTE:onReceive:headset:isInitialStickyBroadcast");
            }

            Log.i(TAG, "AUDIOROUTE:onReceive:" + intent + " isBluetoothConnected=" + isBluetoothConnected());

            Bundle bundle = intent.getExtras();
            if (bundle != null)
            {
                for (String key : bundle.keySet())
                {
                    Log.i(TAG, "AUDIOROUTE:onReceive:intent_dump:" + key + " : " +
                               (bundle.get(key) != null ? bundle.get(key) : "NULL"));
                }
            }

            if ((CallingActivity.activity_state == 1) || (ConferenceAudioActivity.activity_state == 1) ||
                (ConfGroupAudioService.activity_state == 1) || (CallAudioService.activity_state == 1))
            {
                if (intent.getAction().equals("android.intent.action.HEADSET_PLUG"))
                {
                    Log.i(TAG,
                          "AUDIOROUTE:onReceive:" + intent + " state=" + intent.getIntExtra("state", -1) + " name=" +
                          intent.getStringExtra("name") + " mic=" + intent.getIntExtra("microphone", -1));

                    if (intent.getIntExtra("state", -1) == 1)
                    {
                        // headset plugged in
                        Log.i(TAG, "AUDIOROUTE:onReceive:headset:plugged in");
                        set_audio_to_headset(audio_manager_s);
                        Callstate.audio_device = 1;
                        try
                        {
                            update_audio_device_icon();
                        }
                        catch (Exception e2)
                        {
                        }
                        try
                        {
                            update_group_audio_device_icon();
                        }
                        catch (Exception e2)
                        {
                        }
                    }
                    else if (intent.getIntExtra("state", -1) == 0)
                    {
                        // headset unplugged
                        Log.i(TAG, "AUDIOROUTE:onReceive:headset:unplugged");
                        Callstate.audio_device = 0;
                        try
                        {
                            update_audio_device_icon();
                        }
                        catch (Exception e2)
                        {
                        }
                        try
                        {
                            update_group_audio_device_icon();
                        }
                        catch (Exception e2)
                        {
                        }

                        if ((ConferenceAudioActivity.activity_state == 1) ||
                            (ConfGroupAudioService.activity_state == 1))
                        {
                            set_audio_to_loudspeaker(audio_manager_s);
                        }
                        else
                        {
                            if (Callstate.audio_speaker)
                            {
                                set_audio_to_loudspeaker(audio_manager_s);
                            }
                            else
                            {
                                set_audio_to_ear(audio_manager_s);
                            }
                        }
                    }
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        try
        {
            if (intent.getAction().equals("android.media.ACTION_SCO_AUDIO_STATE_UPDATED"))
            {
                Log.i(TAG, "onReceive:" + intent + ":" + intent.getStringExtra("EXTRA_SCO_AUDIO_STATE") + ":" +
                           intent.getStringExtra("EXTRA_SCO_AUDIO_PREVIOUS_STATE"));
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    static boolean isBluetoothConnected()
    {
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter != null && bluetoothAdapter.isEnabled())
        {
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.ICE_CREAM_SANDWICH)
            {
                int a2dpState = bluetoothAdapter.getProfileConnectionState(BluetoothProfile.A2DP);
                int headSetState = bluetoothAdapter.getProfileConnectionState(BluetoothProfile.HEADSET);
                return ((a2dpState == BluetoothAdapter.STATE_CONNECTED ||
                         a2dpState == BluetoothAdapter.STATE_CONNECTING) &&
                        (headSetState == BluetoothAdapter.STATE_CONNECTED ||
                         headSetState == BluetoothAdapter.STATE_CONNECTING));
            }
            else
            {
                return true;
            }
        }
        return false;
    }
}
