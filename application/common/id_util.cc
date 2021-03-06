// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/id_util.h"

#include "base/files/file_path.h"
#include "base/strings/string_util.h"
#include "base/strings/string_number_conversions.h"
#include "crypto/sha2.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest_handlers/tizen_application_handler.h"

#if defined(OS_TIZEN)
#include "third_party/re2/re2/re2.h"
#endif

namespace xwalk {
namespace application {
namespace {
#if defined(OS_TIZEN)
const char kTizenAppIdPattern[] = "\\A[0-9a-zA-Z]{10}[.][0-9a-zA-Z]{1,52}\\z";
const std::string kAppIdPrefix("xwalk.");
#endif
}  // namespace

// Converts a normal hexadecimal string into the alphabet used by applications.
// We use the characters 'a'-'p' instead of '0'-'f' to avoid ever having a
// completely numeric host, since some software interprets that as an IP
// address.
static void ConvertHexadecimalToIDAlphabet(std::string* id) {
  for (size_t i = 0; i < id->size(); ++i) {
    int val;
    if (base::HexStringToInt(base::StringPiece(
            id->begin() + i, id->begin() + i + 1), &val)) {
      (*id)[i] = val + 'a';
    } else {
      (*id)[i] = 'a';
    }
  }
}

// First 16 bytes of SHA256 hashed public key.
const size_t kIdSize = 16;

#if defined(OS_TIZEN)
const size_t kLegacyTizenIdSize = 10;
#endif

std::string GenerateId(const std::string& input) {
  uint8 hash[kIdSize];
  crypto::SHA256HashString(input, hash, sizeof(hash));
  std::string output = StringToLowerASCII(base::HexEncode(hash, sizeof(hash)));
  ConvertHexadecimalToIDAlphabet(&output);

  return output;
}

std::string GenerateIdForPath(const base::FilePath& path) {
  std::string path_bytes =
      std::string(reinterpret_cast<const char*>(path.value().data()),
                  path.value().size() * sizeof(base::FilePath::CharType));
  return GenerateId(path_bytes);
}

#if defined(OS_TIZEN)
std::string RawAppIdToCrosswalkAppId(const std::string& id) {
  if (RE2::PartialMatch(id, kTizenAppIdPattern))
    return GenerateId(id);
  return id;
}

std::string RawAppIdToAppIdForTizenPkgmgrDB(const std::string& id) {
  if (RE2::PartialMatch(id, kTizenAppIdPattern))
    return id;
  return kAppIdPrefix + id;
}

std::string TizenPkgmgrDBAppIdToRawAppId(const std::string& id) {
  std::string raw_id;
  if (RE2::FullMatch(id, "xwalk.(\\w+)", &raw_id))
    return raw_id;
  return id;
}

std::string GetTizenAppId(ApplicationData* application) {
  if (application->GetPackageType() == xwalk::application::Package::XPK)
    return application->ID();

  const TizenApplicationInfo* tizen_app_info =
      static_cast<TizenApplicationInfo*>(application->GetManifestData(
          application_widget_keys::kTizenApplicationKey));
  return tizen_app_info->id();
}
#endif

}  // namespace application
}  // namespace xwalk
