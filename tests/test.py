from invind import Indexer
import pandas as pd

indexer = Indexer()
indexer.add_many_to_many_field("job_category_names")
sample = pd.read_csv("./sample_jcd.csv")
for row in sample.itertuples():
    indexer.add_index(dict(job_category_names=row.job_category_names.split("__")))
qres = indexer.query_execute_batch(
    [dict(job_category_names=dict(contains_all=["法人営業", "営業支援・プリセールス"]))], 3
)
print(sample.iloc[qres[0]])