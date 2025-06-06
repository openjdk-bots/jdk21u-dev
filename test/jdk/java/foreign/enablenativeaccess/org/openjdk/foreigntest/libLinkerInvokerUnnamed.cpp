/*
 * Copyright (c) 2022, 2024, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

#include "jni.h"
#include "testlib_threads.hpp"

void call(void* ctxt) {
    JavaVM* jvm = (JavaVM*) ctxt;
    JNIEnv* env;
    jvm->AttachCurrentThread((void**)&env, nullptr);
    jclass linkerClass = env->FindClass("java/lang/foreign/Linker");
    jmethodID nativeLinkerMethod = env->GetStaticMethodID(linkerClass, "nativeLinker", "()Ljava/lang/foreign/Linker;");
    env->CallStaticVoidMethod(linkerClass, nativeLinkerMethod);
    jvm->DetachCurrentThread();
}

extern "C" {
    JNIEXPORT void JNICALL
    Java_org_openjdk_foreigntest_PanamaMainUnnamedModule_nativeLinker0(JNIEnv *env, jclass cls) {
        JavaVM* jvm;
        env->GetJavaVM(&jvm);
        run_in_new_thread_and_join(call, jvm);
    }
}
