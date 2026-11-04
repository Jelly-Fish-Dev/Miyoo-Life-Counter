#!/bin/sh
# SSH_ASKPASS helper used by the Makefile: answers ssh's password prompt with
# $LC_SSH_PASS (set in local.mk). Prefer key auth (ssh-copy-id) when possible.
printf '%s\n' "$LC_SSH_PASS"
