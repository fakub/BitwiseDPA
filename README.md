# BitwiseDPA

This is a C++ implementation of Bitwise DPA Attack.


### Compile

Run
```sh
$ make
```


### Use

Copy `settings.yaml.template` into `<name-of-your-settings.yaml>` and edit it while following its hints (in comments). Then run
```sh
$ ./main <name-of-your-settings.yaml> > <file-with-results.yaml>
```
where `stdout` is indeed YAML formatted and contains overal results and `stderr` contains some human-readable information.