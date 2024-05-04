# One Billion Row Challenge

Our input is a file called `measurements.txt`,
which contains temperature measurements from various measurement stations.
The file contains exactly one billion rows with the following format:

```
station name;value
station name;value
```

The station name is a UTF-8 string with a maximum length of 100 bytes,
containing any 1-byte or 2-byte characters (except for ';' or '\n').
The measurement values are between -99.9 and 99.9, all with one decimal digit.
The total number of unique stations is limited to 10000.

The output (to stdout) is a lexicographically sorted list of stations,
each with the minimum, average and maximum measured temperature.

```
Abha=-23.0/18.0/59.2
Abidjan=-16.2/26.0/67.3
Abéché=-10.0/29.4/69.0
...
```

## Usage

Install java 21: `nix-shell -p openjdk21`

Clone this repo: https://github.com/gunnarmorling/1brc

And cd into it: `cd 1brc`

Generate `measurements.txt`: `./create_measurements.sh 1000000000`

Generate file with "golden" output: `./calculate_average_baseline.sh | tee golden.txt`

Go back to our repo: `cd ..`

Compile and un code: `make`

## Links

https://simontoth.substack.com/p/daily-bite-of-c-optimizing-code-to

https://github.com/HappyCerberus/1brc

https://github.com/gunnarmorling/1brc?tab=readme-ov-file#running-the-challenge
