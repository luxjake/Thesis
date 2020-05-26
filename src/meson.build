src = [
    'main.cpp',
    'RDF-Parsers/tsv_parser.cpp',
    'RDF-Parsers/shared_dataset_info.cpp',
    'RDF-Parsers/para_parser.cpp',
    'Diffs/basic_diff_result.cpp',
    'Diffs/bitsets_diff.cpp',
    'DBUtils/kyoto_database.cpp',
    'DBUtils/map_database.cpp',
    'DBUtils/multi_kyoto_database.cpp',
    'DBUtils/rocks_database.cpp'
]

src2 = [
    'main_high_diff.cpp',
    'DBUtils/data_manager.cpp',
    'Diffs/high_level_diff.cpp',
    'Diffs/high_diff_result.cpp'
]

src3 = [
    'main_mem_high_diff.cpp',
    'DBUtils/mem_data_manager.cpp',
    'Diffs/mem_high_level_diff.cpp',
    'Diffs/high_diff_result.cpp'
]

executable('lowdiff', sources : src, include_directories : incdir, dependencies : deps)
executable('highdiff', sources : src2, include_directories : incdir, dependencies : deps)
executable('memhighdiff', sources : src3, include_directories : incdir, dependencies : deps)
