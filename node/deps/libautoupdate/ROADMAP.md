# Libautoupdate Roadmap

## v1.0.0

- cater to bad network connection situations
- print more information about the new version
- use better error messages when the user did not have permissions to move the new version into the destination directory
- move the old binary to the system temporary directory, yet not deleting it.
  - the Operating System will delete it when restarted/tmpdir-full
  - add facility to restore/rollback to the old file once the new version went wrong
