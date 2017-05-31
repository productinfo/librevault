/* Copyright (C) 2015 Alexander Shishenko <alex@shishenko.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "Secret.h"
#include "util/AES_CBC_DATA.h"
#include <QList>

namespace librevault {

class Meta {
public:
	enum Type : uint32_t {FILE = 0, DIRECTORY = 1, SYMLINK = 2, /*STREAM = 3,*/ DELETED = 255};
	enum AlgorithmType : uint8_t {RABIN=0};
	enum StrongHashType : uint8_t {SHA3_256=0};
	struct Chunk {
		QByteArray ct_hash;
		uint32_t size;
		QByteArray iv;

		QByteArray pt_hmac;

		static QByteArray encrypt(QByteArray chunk, QByteArray key, QByteArray iv);
		static QByteArray decrypt(QByteArray chunk, uint32_t size, QByteArray key, QByteArray iv);

		static QByteArray compute_strong_hash(QByteArray chunk, StrongHashType type);
	};

	struct RabinGlobalParams {
		uint64_t polynomial = 0x3DA3358B4DC173LL;
		uint32_t polynomial_degree = 53;
		uint32_t polynomial_shift = 53 - 8;
		uint32_t avg_bits = 20;
	};

private:
	/* Meta fields, must be serialized together and then signed */

	/* Required data */
	QByteArray path_id_;
	AES_CBC_DATA path_;
	Type meta_type_ = FILE;
	int64_t revision_ = 0;	// timestamp of Meta modification

	/* Content-specific metadata */
	// Generic metadata
	int64_t mtime_ = 0;	// file/directory mtime
	/// Windows-specific
	uint32_t windows_attrib_ = 0;
	/// Unix-specific
	uint32_t mode_ = 0;
	uint32_t uid_ = 0;
	uint32_t gid_ = 0;

	// Symlink metadata
	AES_CBC_DATA symlink_path_;

	// File metadata
	/// Algorithm selection
	AlgorithmType algorithm_type_ = (AlgorithmType)0;
	StrongHashType strong_hash_type_ = (StrongHashType)0;

	/// Uni-algorithm parameters
	uint32_t max_chunksize_ = 0;
	uint32_t min_chunksize_ = 0;

	/// Rabin algorithm parameters
	AES_CBC_DATA rabin_global_params_;

	QList<Chunk> chunks_;

public:
	/* Nested structs & classes */
	struct error : std::runtime_error {
		error(const char* what) : std::runtime_error(what) {}
		error() : error("Meta error") {}
	};

	struct parse_error : error {
		parse_error(const char* what) : error(what) {}
		parse_error() : error("Parse error") {}
	};

	/// Used for querying specific version of Meta
	struct PathRevision {
		QByteArray path_id_;
		int64_t revision_;
	};

	/* Class methods */
	Meta();
	explicit Meta(QByteArray meta_s);
	virtual ~Meta();

	/* Serialization */
	QByteArray serialize() const;
	void parse(QByteArray serialized_data);

	/* Validation */
	bool validate() const;
	//bool validate(const Secret& secret) const;

	/* Generators */
	static QByteArray makePathId(QString path, const Secret& secret);

	/* Smart getters+setters */
	PathRevision path_revision() const {return PathRevision{pathId(), revision()};}
	uint64_t size() const;

	// Encryptors/decryptors
	QByteArray path(const Secret& secret) const;
	void setPath(QByteArray path, const Secret& secret);   // Also, computes and sets path_id

	QByteArray symlinkPath(const Secret& secret) const;
	void setSymlinkPath(QByteArray path, const Secret& secret);

	RabinGlobalParams rabin_global_params(const Secret& secret) const;
	void set_rabin_global_params(const RabinGlobalParams& rabin_global_params, const Secret& secret);
	AES_CBC_DATA& raw_rabin_global_params() {return rabin_global_params_;}
	const AES_CBC_DATA& raw_rabin_global_params() const {return rabin_global_params_;}

	// Dumb getters & setters
	QByteArray pathId() const {return path_id_;}
	void setPathId(QByteArray path_id) {path_id_ = path_id;}

	Type meta_type() const {return meta_type_;}
	void set_meta_type(Type meta_type) {meta_type_ = meta_type;}

	int64_t revision() const {return revision_;}
	void set_revision(int64_t revision) {revision_ = revision;}

	int64_t mtime() const {return mtime_;}
	void set_mtime(int64_t mtime) {mtime_ = mtime;}

	uint32_t windows_attrib() const {return windows_attrib_;}
	void set_windows_attrib(uint32_t windows_attrib) {windows_attrib_ = windows_attrib;}

	uint32_t mode() const {return mode_;}
	void set_mode(uint32_t mode) {mode_ = mode;}

	uint32_t uid() const {return uid_;}
	void set_uid(uint32_t uid) {uid_ = uid;}

	uint32_t gid() const {return gid_;}
	void set_gid(uint32_t gid) {gid_ = gid;}

	AlgorithmType algorithm_type() const {return algorithm_type_;}
	void set_algorithm_type(AlgorithmType algorithm_type) {algorithm_type_ = algorithm_type;}

	StrongHashType strong_hash_type() const {return strong_hash_type_;}
	void set_strong_hash_type(StrongHashType strong_hash_type) {strong_hash_type_ = strong_hash_type;}

	uint32_t min_chunksize() const {return min_chunksize_;}
	void set_min_chunksize(uint32_t min_chunksize) {min_chunksize_ = min_chunksize;}

	uint32_t max_chunksize() const {return max_chunksize_;}
	void set_max_chunksize(uint32_t max_chunksize) {max_chunksize_ = max_chunksize;}

	QList<Chunk> chunks() const {return chunks_;}
	void set_chunks(QList<Chunk> chunks) {chunks_ = chunks;}
};

} /* namespace librevault */