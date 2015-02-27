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

#define GLM_FORCE_RADIANS

#include <jni.h>

#include "tango_client_api.h"

#include "tango-gl/camera.h"
#include "tango-gl/grid.h"
#include "tango-gl/util.h"

GLuint screen_width;
GLuint screen_height;

// Render camera.
tango_gl::Camera* cam;

// Ground grid.
tango_gl::Grid* grid;

// Camera vertical offset value.
const glm::vec3 kHeightOffset = glm::vec3(0.0f, 1.3f, 0.0f);

// FOV set up values.
const float kHighFov = 65.0f;

TangoConfig config_;

glm::quat tango_rotation;
glm::vec3 tango_position;

bool SetupGraphics(int w, int h) {
    screen_width = w;
    screen_height = h;

    tango_position = glm::vec3(0.0f, 0.0f, 0.0f);
    tango_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    cam = new tango_gl::Camera();
    grid = new tango_gl::Grid();

    if (h == 0) {
      LOGE("Setup graphic height not valid");
      return false;
    }
    cam->SetAspectRatio(static_cast<float>(w) / static_cast<float>(h));
    return true;
}

// Render current frame.
bool RenderFrame() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, screen_width, screen_height);

    glm::vec3 position = tango_gl::util::ConvertPositionToOpenGL(tango_position);

    position += kHeightOffset;

    glm::quat rotation = tango_gl::util::ConvertRotationToOpenGL(tango_rotation);

    cam->SetPosition(position);
    cam->SetRotation(rotation);

    grid->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());

    return true;
}

void onPoseAvailable(void*, const TangoPoseData* pose){
    tango_position = glm::vec3(pose->translation[0], pose->translation[1],
                    pose->translation[2]);
    tango_rotation = glm::quat(pose->orientation[3], pose->orientation[0],
                    pose->orientation[1], pose->orientation[2]);
    return;
}

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_tangoInitialize(
      JNIEnv* env, jobject, jobject activity) {
    return TangoService_initialize(env, activity);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_tangoSetupConfig(
      JNIEnv*, jobject) {
    config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
    if (config_ == NULL) {
        LOGE("TangoService_getConfig(): Failed");
    }
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_tangoConnectCallbacks(
      JNIEnv*, jobject) {
    TangoCoordinateFramePair pairs;
    pairs.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
    pairs.target = TANGO_COORDINATE_FRAME_DEVICE;

    return TangoService_connectOnPoseAvailable(1, &pairs, onPoseAvailable);
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_tangoConnect(
      JNIEnv*, jobject) {
    return TangoService_connect(nullptr, config_);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_tangoDisconnect(
      JNIEnv*, jobject) {
    TangoConfig_free(config_);
    config_ = NULL;
    TangoService_disconnect();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_freeGLContent(
    JNIEnv*, jobject) {
    delete cam;
    delete grid;
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_setupGraphic(
    JNIEnv*, jobject, jint width, jint height) {
    SetupGraphics(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativemotiontracking_TangoJNINative_render(
    JNIEnv*, jobject) {
    RenderFrame();
}

#ifdef __cplusplus
}
#endif
