// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#pragma once

#include <gen_cpp/DataSinks_types.h>

#include "io/fs/file_writer.h"
#include "vec/columns/column.h"
#include "vec/exprs/vexpr_fwd.h"
#include "vec/runtime/vfile_format_transformer.h"

namespace doris {

class ObjectPool;
class RuntimeState;
class RuntimeProfile;

namespace vectorized {

class Block;
class VFileFormatTransformer;

class VHivePartitionWriter {
public:
    struct WriteInfo {
        std::string write_path;
        std::string target_path;
        TFileType::type file_type;
    };

    VHivePartitionWriter(const TDataSink& t_sink, const std::string partition_name,
                         TUpdateMode::type update_mode, const VExprContextSPtrs& output_expr_ctxs,
                         const std::vector<THiveColumn>& columns, WriteInfo write_info,
                         const std::string file_name, TFileFormatType::type file_format_type,
                         TFileCompressType::type hive_compress_type,
                         const std::map<std::string, std::string>& hadoop_conf);

    Status init_properties(ObjectPool* pool) { return Status::OK(); }

    Status open(RuntimeState* state, RuntimeProfile* profile);

    Status write(vectorized::Block& block, IColumn::Filter* filter = nullptr);

    Status close(Status);

    inline size_t written_len() { return _vfile_writer->written_len(); }

private:
    std::unique_ptr<orc::Type> _build_orc_type(const TypeDescriptor& type_descriptor);

    Status _projection_and_filter_block(doris::vectorized::Block& input_block,
                                        const vectorized::IColumn::Filter* filter,
                                        doris::vectorized::Block* output_block);

    THivePartitionUpdate _build_partition_update();

    std::string _path;

    std::string _partition_name;

    TUpdateMode::type _update_mode;

    size_t _row_count = 0;
    size_t _input_size_in_bytes = 0;

    const VExprContextSPtrs& _vec_output_expr_ctxs;

    const std::vector<THiveColumn>& _columns;
    WriteInfo _write_info;
    std::string _file_name;
    TFileFormatType::type _file_format_type;
    TFileCompressType::type _hive_compress_type;
    const std::map<std::string, std::string>& _hadoop_conf;

    // If the result file format is plain text, like CSV, this _file_writer is owned by this FileResultWriter.
    // If the result file format is Parquet, this _file_writer is owned by _parquet_writer.
    std::unique_ptr<doris::io::FileWriter> _file_writer_impl = nullptr;
    // convert block to parquet/orc/csv format
    std::unique_ptr<VFileFormatTransformer> _vfile_writer = nullptr;

    RuntimeState* _state;
};
} // namespace vectorized
} // namespace doris
