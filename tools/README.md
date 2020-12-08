# Purpose

The scripts in this folder are useful for developers of the RStudio Launcher Plugin SDK when implementing and testing features for the SDK.
Please be advised that these scripts may modify your enviroment by adding or removing users.
Administrator privileges may be required and users should ensure that they understand the impact of running the scripts before running them.

## create-test-users.sh

Usage: `./create-test-users.sh [0|1]?`

This script will create the five users and three user groups which are required in order for the unit tests of the RStudio Launcher Plugin SDK to pass.
The users will have home directories under default `$HOME` root, and are as follows:

* `rlpstestusrone`
* `rlpstestusrtwo`
* `rlpstestusrthree`
* `rlpstestusrfour`
* `rlpstestusrfive`

The user groups, and the users in them, are defined as follows:

* `rlpstestgrpone`: `rlpstestusrone`, `rlpstestusrtwo`, `rlpstestusrfive`
* `rlpstestgrptwo`: `rlpstestusrtwo`, `rlpstestusrthree`, `rlpstestusrfour`
* `rlpstestgrpthree`: `rlpstestusrtwo`, `rlpstestusrfour`, `rlpstestusrfive`

It also adds the system user, `rstudio-server`, if requested.
To request the creation of the `rstudio-server` user, pass `1` as a parameter when invoking the script.

## delete-test-users.sh

Usage: `./delete-test-users.sh [0|1]?`

This script will delete all of the users and groups created by the `create-test-user.sh` script.
It will also destroy their home directories and all the contents within them.

Optionally, this script may destroy the system user, `rstudio-server`, if requested.
To request the deletion of the `rstudio-server` user, pass `1` as a parameter when invoking the script.
The `rstudio-server` user should only be removed if it was created by the `create-test-user.sh` script.

## run-all-tests.sh

Usage: `./run-all-tests.sh <path to build directory root>`

This script can be used to run all of the unit tests provided with the RStudio Launcher Plugin SDK.
It will take the following actions:

1. Detect whether the `rstudio-server` system user exists.
2. Create all the test users, including the `rstudio-server` user if it did not exist already.
3. Run each of the unit tests, emitting output inidicating the success or failure of the tests.
4. Delete all of the users it created in step 2, including the `rstudio-server` user, if it was created.
5. Output the total number of failed tests.