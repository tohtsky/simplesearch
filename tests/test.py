import sys
from invind import Indexer

indexer = Indexer()
indexer.add_categorical_field("hoge").add_categorical_field("job")

indexer.add_index(dict(hoge="1", job="J01"))
indexer.add_index(dict(hoge="1", job="J02"))
indexer.add_index(dict(hoge="2", job="J01"))

print(indexer.query_execute_batch([dict(hoge="1"), dict(hoge=["1", "2"])], 3))
