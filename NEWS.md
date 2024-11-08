RStudio Launcher Plugin Software Development Kit 1.1.4
--------------------------------------------------------------------------------------------

### New
* Additional check to see if job is still running before pruning. (#76)

* Log errors when tailing job output, instead of reporting the error, to avoid  closing the stream. (#79)
  

RStudio Launcher Plugin Software Development Kit 1.1.3
--------------------------------------------------------------------------------------------

### New
* The main `syslog` log destination for plugins has been changed to a `file` log destination, causing logs to be written to file now by default. Logs are written to the `logging-dir` directory. By default, this is located at `/var/log/rstudio/launcher`.
  
* The `logging-dir` flag passed by the Launcher is now supported and determines where the main file log destination is created. By default, this is located at `/var/log/rstudio/launcher`.
