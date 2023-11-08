/* Copyright 2017 - 2023 R. Thomas
 * Copyright 2017 - 2023 Quarkslab
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LIEF_ELF_NOTE_H
#define LIEF_ELF_NOTE_H

#include <vector>
#include <ostream>
#include <memory>

#include "LIEF/Object.hpp"
#include "LIEF/visibility.h"
#include "LIEF/errors.hpp"

#include "LIEF/ELF/enums.hpp"

namespace LIEF {
class BinaryStream;
namespace ELF {

/// Class which represents an ELF note. This class can be instantiated using
/// the static Note::create functions.
class LIEF_API Note : public Object {
  friend class Parser;
  friend class Builder;
  friend class Binary;

  public:
  //! Container used to handle the description data
  using description_t = std::vector<uint8_t>;

  /// LIEF representation of the ELF `NT_` values.
  enum class TYPE {
    UNKNOWN = 0,
    /// Match `NT_GNU_ABI_TAG`: Operating system (OS) ABI information
    ///
    /// See: NoteAbi
    GNU_ABI_TAG,
    /// Match `NT_HWCAP`: Synthetic hardware capabilities information
    GNU_HWCAP,
    /// Match `NT_GNU_BUILD_ID`: Unique build ID as generated by the GNU ld
    GNU_BUILD_ID,
    /// Match `NT_GNU_GOLD_VERSION`: The version of gold used to link
    GNU_GOLD_VERSION,
    /// Match `NT_GNU_PROPERTY_TYPE_0`: Program property note, as described in
    /// "Linux Extensions to the gABI".
    GNU_PROPERTY_TYPE_0,

    GNU_BUILD_ATTRIBUTE_OPEN,
    GNU_BUILD_ATTRIBUTE_FUNC,

    /// Crashpad note used by the Chromium project
    CRASHPAD,

    /// Coredump that wraps the `elf_prstatus` structure
    CORE_PRSTATUS,
    CORE_FPREGSET,
    /// Coredump that wraps the `elf_prpsinfo` structure
    ///
    /// See: CorePrPsInfo
    CORE_PRPSINFO,
    CORE_TASKSTRUCT,
    /// Coredump that contains a copy of all the auxiliary vectors (auxv)
    ///
    /// See: CoreAuxv
    CORE_AUXV,
    CORE_PSTATUS,
    /// Coredump that wraps the `fpregset` structure
    CORE_FPREGS,
    /// Coredump that wraps the `psinfo` structure
    CORE_PSINFO,
    CORE_LWPSTATUS,
    CORE_LWPSINFO,
    CORE_WIN32PSTATUS,
    CORE_FILE,
    CORE_PRXFPREG,
    CORE_SIGINFO,

    CORE_ARM_VFP,
    CORE_ARM_TLS,
    CORE_ARM_HW_BREAK,
    CORE_ARM_HW_WATCH,
    CORE_ARM_SYSTEM_CALL,
    CORE_ARM_SVE,
    CORE_ARM_PAC_MASK,
    CORE_ARM_PACA_KEYS,
    CORE_ARM_PACG_KEYS,
    CORE_TAGGED_ADDR_CTRL,
    CORE_PAC_ENABLED_KEYS,

    CORE_X86_TLS,
    CORE_X86_IOPERM,
    CORE_X86_XSTATE,
    CORE_X86_CET,

    /// Note that is specific to Android and that describes information such as
    /// the NDK version or the SDK build number.
    ///
    /// See AndroidIdent
    ANDROID_IDENT,
    ANDROID_MEMTAG,
    ANDROID_KUSER,

    /// Note specific to Go binaries
    GO_BUILDID,
    /// Note for SystemTap probes
    STAPSDT,
  };

  public:
  /// Convert the raw integer note type into a TYPE according to the owner
  static result<TYPE> convert_type(E_TYPE ftype, uint32_t type,
                                   const std::string& name);

  /// Try to determine the ELF section name associated with the TYPE
  /// provided in parameter
  static result<const char*> type_to_section(TYPE type);

  /// Try to determine the owner's name of the TYPE provided in parameter
  static result<const char*> type_owner(TYPE type);

  /// Create a new note from the given parameters. Additional information
  /// such as the architecture or the ELF class could be required for
  /// creating notes like Coredump notes.
  static std::unique_ptr<Note> create(
      const std::string& name, uint32_t type, description_t description,
      E_TYPE ftype = E_TYPE::ET_NONE, ARCH arch = ARCH::EM_NONE,
      ELF_CLASS cls = ELF_CLASS::ELFCLASSNONE);

  /// Create a new note from the given parameters. Additional information
  /// such as the architecture or the ELF class could be required for
  /// creating notes like Coredump notes.
  static std::unique_ptr<Note> create(
      const std::string& name, TYPE type, description_t description,
      ARCH arch = ARCH::EM_NONE, ELF_CLASS cls = ELF_CLASS::ELFCLASSNONE);

  /// Create a new note from the given stream. Additional information
  /// such as the architecture or the ELF class could be required for
  /// creating notes like Coredump notes.
  static std::unique_ptr<Note> create(BinaryStream& stream,
      E_TYPE ftype = E_TYPE::ET_NONE, ARCH arch = ARCH::EM_NONE,
      ELF_CLASS cls = ELF_CLASS::ELFCLASSNONE);

  Note& operator=(const Note& copy) = default;
  Note(const Note& copy) = default;

  ~Note() override = default;

  /// Clone the current note and keep its polymorphic type
  virtual std::unique_ptr<Note> clone() const {
    return std::unique_ptr<Note>(new Note(*this));
  }

  /// Return the *name* of the note (also known as 'owner' )
  const std::string& name() const {
    return name_;
  }

  /// Return the type of the note. This type does not match the `NT_` type
  /// value. For accessing the original `NT_` value, check original_type()
  TYPE type() const {
    return type_;
  }

  /// The original `NT_xxx` integer value. The meaning of this value likely
  /// depends on the owner of the note.
  uint32_t original_type() const {
    return original_type_;
  }

  //! Return the description associated with the note
  const description_t& description() const {
    return description_;
  }

  description_t& description() {
    return description_;
  }

  void name(std::string name) {
    name_ = std::move(name);
  }

  /// Change the description of the note
  void description(description_t description) {
    description_ = std::move(description);
  }

  /// Size of the **raw** note which includes padding
  uint64_t size() const;

  virtual void dump(std::ostream& os) const;

  void accept(Visitor& visitor) const override;

  LIEF_API friend
  std::ostream& operator<<(std::ostream& os, const Note& note) {
    note.dump(os);
    return os;
  }

  protected:
  Note() = default;
  Note(std::string name, TYPE type, uint32_t original_type,
       description_t description) :
    name_(std::move(name)),
    type_(type),
    original_type_(original_type),
    description_(std::move(description))
  {}

  template<class T>
  LIEF_LOCAL result<T> read_at(size_t offset) const;

  template<class T>
  LIEF_LOCAL ok_error_t write_at(size_t offset, const T& value);

  LIEF_LOCAL ok_error_t write_string_at(size_t offset, const std::string& value);

  LIEF_LOCAL result<std::string>
  read_string_at(size_t offset, size_t maxsize = 0) const;

  std::string  name_;
  TYPE type_ = TYPE::UNKNOWN;
  uint32_t original_type_ = 0;
  description_t description_;
};

LIEF_API const char* to_string(Note::TYPE type);


} // namepsace ELF
} // namespace LIEF
#endif
