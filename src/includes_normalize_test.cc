// Copyright 2012 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "includes_normalize.h"

#include <algorithm>

#include <direct.h>

#include "string_piece_util.h"
#include "test.h"
#include "util.h"

namespace {

string GetCurDir() {
  char buf[_MAX_PATH];
  _getcwd(buf, sizeof(buf));
  vector<StringPiece> parts = SplitStringPiece(buf, '\\');
  return parts[parts.size() - 1].AsString();
}

string NormalizeAndCheckNoError(const string& input) {
  string result, err;
  IncludesNormalize normalizer(".");
  EXPECT_TRUE(normalizer.Normalize(input, &result, &err));
  EXPECT_EQ("", err);
  return result;
}

string NormalizeRelativeAndCheckNoError(const string& input,
                                        const string& relative_to) {
  string result, err;
  IncludesNormalize normalizer(relative_to);
  EXPECT_TRUE(normalizer.Normalize(input, &result, &err));
  EXPECT_EQ("", err);
  return result;
}

}  // namespace

TEST(IncludesNormalize, Simple) {
  EXPECT_EQ("b", NormalizeAndCheckNoError("a\\..\\b"));
  EXPECT_EQ("b", NormalizeAndCheckNoError("a\\../b"));
  EXPECT_EQ("a/b", NormalizeAndCheckNoError("a\\.\\b"));
  EXPECT_EQ("a/b", NormalizeAndCheckNoError("a\\./b"));
}

TEST(IncludesNormalize, WithRelative) {
  string currentdir = GetCurDir();
  EXPECT_EQ("c", NormalizeRelativeAndCheckNoError("a/b/c", "a/b"));
  EXPECT_EQ("a", NormalizeAndCheckNoError(IncludesNormalize::AbsPath("a")));
  EXPECT_EQ(string("../") + currentdir + string("/a"),
            NormalizeRelativeAndCheckNoError("a", "../b"));
  EXPECT_EQ(string("../") + currentdir + string("/a/b"),
            NormalizeRelativeAndCheckNoError("a/b", "../c"));
  EXPECT_EQ("../../a", NormalizeRelativeAndCheckNoError("a", "b/c"));
  EXPECT_EQ(".", NormalizeRelativeAndCheckNoError("a", "a"));
}

TEST(IncludesNormalize, Case) {
  EXPECT_EQ("b", NormalizeAndCheckNoError("Abc\\..\\b"));
  EXPECT_EQ("BdEf", NormalizeAndCheckNoError("Abc\\..\\BdEf"));
  EXPECT_EQ("A/b", NormalizeAndCheckNoError("A\\.\\b"));
  EXPECT_EQ("a/b", NormalizeAndCheckNoError("a\\./b"));
  EXPECT_EQ("A/B", NormalizeAndCheckNoError("A\\.\\B"));
  EXPECT_EQ("A/B", NormalizeAndCheckNoError("A\\./B"));
}

TEST(IncludesNormalize, DifferentDrive) {
  EXPECT_EQ("stuff.h",
            NormalizeRelativeAndCheckNoError("p:\\vs08\\stuff.h", "p:\\vs08"));
  EXPECT_EQ("stuff.h",
            NormalizeRelativeAndCheckNoError("P:\\Vs08\\stuff.h", "p:\\vs08"));
  EXPECT_EQ("p:/vs08/stuff.h",
            NormalizeRelativeAndCheckNoError("p:\\vs08\\stuff.h", "c:\\vs08"));
  EXPECT_EQ("P:/vs08/stufF.h", NormalizeRelativeAndCheckNoError(
                                   "P:\\vs08\\stufF.h", "D:\\stuff/things"));
  EXPECT_EQ("P:/vs08/stuff.h", NormalizeRelativeAndCheckNoError(
                                   "P:/vs08\\stuff.h", "D:\\stuff/things"));
  EXPECT_EQ("P:/wee/stuff.h",
            NormalizeRelativeAndCheckNoError("P:/vs08\\../wee\\stuff.h",
                                             "D:\\stuff/things"));
}

TEST(IncludesNormalize, LongInvalidPath) {
  const char kLongInputString[] =
      "C:\\Program Files (x86)\\Microsoft Visual Studio "
      "12.0\\VC\\INCLUDEwarning #31001: The dll for reading and writing the "
      "pdb (for example, mspdb110.dll) could not be found on your path. This "
      "is usually a configuration error. Compilation will continue using /Z7 "
      "instead of /Zi, but expect a similar error when you link your program.";
  // Too long, won't be canonicalized. Ensure doesn't crash.
  string result, err;
  IncludesNormalize normalizer(".");
  EXPECT_FALSE(
      normalizer.Normalize(kLongInputString, &result, &err));
  EXPECT_EQ("path too long", err);

  const char kExactlyMaxPath[] =
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "012345678\\"
      "0123456789";
  string forward_slashes(kExactlyMaxPath);
  replace(forward_slashes.begin(), forward_slashes.end(), '\\', '/');
  // Make sure a path that's exactly _MAX_PATH long is canonicalized.
  EXPECT_EQ(forward_slashes,
            NormalizeAndCheckNoError(kExactlyMaxPath));
}
