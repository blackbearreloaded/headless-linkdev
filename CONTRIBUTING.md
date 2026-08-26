# Contributing

Thanks for helping improve Headless LinkDev. The project intentionally
stays small: one payload, one connection, and no UI dependencies.

## Before opening a pull request

1. Keep the change focused and do not commit generated ELF files or SDK files.
2. Run the host self-test and formatting check:

   ```sh
   make test PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
   make format-check PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
   ```

3. Build the PS5 payload:

   ```sh
   make PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
   ```

4. For PS5-facing changes, report the tested firmware, payload environment,
   and chiaki-ng pairing result.
5. Update the README and third-party notices when behavior or attribution
   changes.

## Pull-request checklist

- [ ] Host self-test passes.
- [ ] Formatting check passes.
- [ ] PS5 validation is included or explicitly marked as not run.
- [ ] Documentation matches the payload behavior.
- [ ] No generated artifacts or SDK files are staged.
