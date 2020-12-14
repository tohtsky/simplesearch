## simplesearch

This Python module is intended to perform simple searches on categorical variables (& floating point variables) in-memory at high speed, using inverted index.

## Example

```Python
from simplesearch import Indexer

DATA = [
    {
        "age": 30,
        "gender": "M",
        "job_categories": ["1", "2", "3"],
    },  # index 0
    {"age": 35, "gender": "F", "job_categories": ["1"]},  # index 1
    {"age": 40, "gender": "M", "job_categories": None},  # index 2
    {
        "age": None,
        "gender": None,
        "job_categories": ["2", "3"],
    },  # index 3
]
indexer = (
    Indexer()
    .add_categorical_field("gender")
    .add_categorical_field("job_categories")
    .add_numerical_field("age")
)
for row in DATA:
    indexer.add_index(row)

print(indexer.query_execute(dict(gender="M")))  # [0 2]
print(indexer.query_execute({"job_categories": "1"}))  # [0 1]
print(indexer.query_execute({"job_categories": ["1"]}))  # an equivalent of the above
print(
    indexer.query_execute({"job_categories": {"contains_one": ["1"]}})
)  # an equivalent of the above
print(
    indexer.query_execute({"job_categories": {"contains_one": ["1", "2"]}})
)  # [0 1 3]
print(indexer.query_execute({"job_categories": {"contains_all": ["1", "2"]}}))  # [0]

print(
    indexer.query_execute(
        {"age": {"lte": 33}, "job_categories": {"contains_one": ["1", "2"]}}
    )
)  # [0]
print(
    indexer.query_execute(
        {
            "age": {"lt": 40},
            "gender": ["M", "F"],
            "job_categories": {"contains_one": ["1", "2", "3"]},
        }
    )
)  # [0 1]

# multi-threaded batch execution of queries
print(
    indexer.query_execute_batch(
        [
            {"job_categories": "1"},
            {"job_categories": {"contains_one": ["1", "2"]}},
            {
                "age": {"lt": 40},
                "gender": ["M", "F"],
                "job_categories": {"contains_one": ["1", "2", "3"]},
            },
        ],
        n_workers=3,
    ),
)
```
