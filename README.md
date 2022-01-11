# DiskAnalyzer
A Linux daemon that analyzes used disk space starting from a given path. 

Multiple analysis tasks with differing priorities can be started at the same time. Each analysis task can be suspended, resumed or removed.
The **da** command-line tool is used to send commands to the daemon.  

## Installation
1. Download the *diskanalyzer.tar* archive from this repository
2. Run the following commands:
```bash
tar xvf diskanalyzer.tar
sudo ./da_installer.sh
```
3. This will compile the source files and move the **da** binary to /usr/local/bin, and the daemon and job executables to /opt/diskanalyzer
4. You can then delete the archive, source files and installer

## Usage
Use `da start` to start the daemon, then refer to `da -h` for instructions on sending commands to the daemon.

Flags:
```txt
 -a, --add <dirpath>     start new analysis job on directory <dirpath>
 -p, --priority <1-3>    set job priority (only after -a flag), 
 -l, --list              print status of all jobs
 -i, --info <id>         print status of job with the given <id> 
 -p, --print <id>        print analysis results of job with the given <id>
 -r, --remove <id>       remove task with the given <id>
 -S, --suspend <id>      suspend task with the given <id>
 -R, --resume <id>       resume task with the given <id>
```

## Examples

#### Starting the daemon
```bash
da start
```

#### Starting a job
```bash
> da -a /home

Started new analysis job
ID = 3
Directory = /home

```

#### Printing results of job with id 3
```bash
da -p 3
```

#### Listing status of all jobs 
```bash
> da -l

Job  Prio    Path    Done%   Status       	 	 Details 

ID 1 **  /home/aidan -> 100% DONE 	 52964 files 6409 directories

ID 2 *  /usr -> 100% DONE 	 377266 files 39904 directories

ID 3 *  /home -> 100% DONE 	 52964 files 6411 directories

```

## Implementation details
This software was written by a team of four students, as a final project for the **Operating Systems** course at the University of Bucharest.

We gained experience in using git as a team and a deeper understanding of daemon functionality, inter-process communication and Unix-like operating systems.
