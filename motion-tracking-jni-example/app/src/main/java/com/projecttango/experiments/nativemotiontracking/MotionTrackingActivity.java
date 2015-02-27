/*
 * Copyright 2014 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.projecttango.experiments.nativemotiontracking;

import android.app.Activity;
import android.content.Intent;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;

/**
 * Main activity shows motion tracking scene.
 */
public class MotionTrackingActivity extends Activity {
    private static final int TANGO_NO_MOTION_TRACKING_PERMISSION = -3;
    /// The input argument is invalid.
    private static final int TANGO_INVALID = -2;
    /// This error code denotes some sort of hard error occurred.
    private static final int TANGO_ERROR = -1;
    /// This code indicates success.
    private static final int TANGO_SUCCESS = 0;

    public static final String EXTRA_KEY_PERMISSIONTYPE = "PERMISSIONTYPE";
    public static final String EXTRA_VALUE_MOTION_TRACKING =
            "MOTION_TRACKING_PERMISSION";

    private static final String TAG =
            MotionTrackingActivity.class.getSimpleName();

    private MotionTrackingRenderer mRenderer;
    private GLSurfaceView mGLView;

    private boolean mIsPermissionGranted = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_motion_tracking);

        // OpenGL view where all of the graphics are drawn
        mGLView = (GLSurfaceView) findViewById(R.id.gl_surface_view);

        // Configure OpenGL renderer
        mRenderer = new MotionTrackingRenderer();
        mGLView.setRenderer(mRenderer);

        if (TangoJNINative.tangoInitialize(this) != TANGO_SUCCESS) {
            Log.e(TAG, "tangoInitialize error");
        }
    }

    @Override
    protected void onResume() {
        super.onPause();
        mGLView.onResume();

        int err = 0;
        err = TangoJNINative.tangoConnectCallbacks();
        if (err == TANGO_NO_MOTION_TRACKING_PERMISSION) {
            Log.i(TAG, "no permission granted call intent");
            callPermissionIntent();
        } else if (err == TANGO_INVALID) {
            Log.e(TAG, "tango initialize invalid");
        } else if (err == TANGO_ERROR) {
            Log.e(TAG, "tango initialize error");
        } else if (err == TANGO_SUCCESS) {
            mIsPermissionGranted = true;
            TangoJNINative.tangoSetupConfig();
            if (TangoJNINative.tangoConnect() != TANGO_SUCCESS) {
               Log.e(TAG, "tango connect error");
            }
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        mGLView.onPause();
        if (mIsPermissionGranted) {
            TangoJNINative.tangoDisconnect();
        }
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    private void callPermissionIntent() {
        Intent intent = new Intent();
        intent.setAction("android.intent.action.REQUEST_TANGO_PERMISSION");
        intent.putExtra(EXTRA_KEY_PERMISSIONTYPE, EXTRA_VALUE_MOTION_TRACKING);
        startActivityForResult(intent, 0);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == 0) {
            if (resultCode == RESULT_CANCELED) {
                finish();
            } else {
                mIsPermissionGranted = true;
            }
        }
    }
}
