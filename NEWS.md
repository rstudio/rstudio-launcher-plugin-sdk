RStudio Launcher Plugin Software Development Kit 1.1.3
--------------------------------------------------------------------------------------------

### New
* The main `syslog` log destination for plugins has been changed to a `file` log destination, causing logs to be written to file now by default. Logs are written to the `logging-dir` directory. By default, this is located at `/var/log/rstudio/launcher`.
  
* The `log-dir` flag is now supported and creates the main file log destination.
  This is by defualt `/var/log/rstudio/launcher`.
