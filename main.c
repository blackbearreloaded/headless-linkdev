// SPDX-License-Identifier: GPL-3.0-or-later
// Headless stdout pairing payload based on LinkDev and ActRemoteLink.

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef SELF_TEST
#include <assert.h>
#endif

#define ACCOUNT_SLOT_COUNT 16
#define PAIRING_TIMEOUT_SECONDS 300
#define LOG_INTERVAL_SECONDS 60

#define REG_KEY_REMOTEPLAY_ENABLED 1098973184
#define REG_SLOT_STRIDE 65536
#define REG_KEY_USER_ID_FIRST 125829376
#define REG_KEY_USER_ID_INVALID 127140096
#define REG_KEY_ACCOUNT_ID_FIRST 125830400
#define REG_KEY_ACCOUNT_ID_INVALID 127141120
#define REG_KEY_USER_REMOTEPLAY_FIRST 125859841
#define REG_KEY_USER_REMOTEPLAY_INVALID 127170561

#define PAIR_ERROR_INVALID_ACCOUNT ((int)0x80fc1040)
#define PAIR_ERROR_INVALID_PIN ((int)0x80fc1047)

#ifndef SELF_TEST
int sceUserServiceInitialize(void *);
int sceUserServiceGetForegroundUser(int *);

int sceRegMgrGetInt(int, int *);
int sceRegMgrGetBin(int, void *, size_t);
int sceRegMgrSetInt(int, int);

int sceRemoteplayInitialize(void *, size_t);
int sceRemoteplayGeneratePinCode(uint32_t *);
int sceRemoteplayConfirmDeviceRegist(int *, int *);
int sceRemoteplayNotifyPinCodeError(int);

static void log_message(const char *message)
{
    printf("[RemotePlayPair] %s\n", message);
    fflush(stdout);
}
#endif

static int slot_key(int slot, int first_key, int invalid_key)
{
    if (slot < 1 || slot > ACCOUNT_SLOT_COUNT)
    {
        return invalid_key;
    }

    return first_key + ((slot - 1) * REG_SLOT_STRIDE);
}

#ifndef SELF_TEST
static int current_user_slot(void)
{
    int foreground_user = 0;

    if (sceUserServiceInitialize(0) != 0 || sceUserServiceGetForegroundUser(&foreground_user) != 0)
    {
        return -1;
    }

    for (int slot = 1; slot <= ACCOUNT_SLOT_COUNT; slot++)
    {
        int user_id = 0;
        int key = slot_key(slot, REG_KEY_USER_ID_FIRST, REG_KEY_USER_ID_INVALID);

        if (sceRegMgrGetInt(key, &user_id) == 0 && user_id == foreground_user)
        {
            return slot;
        }
    }

    return -1;
}
#endif

static void base64_encode_account_id(const uint8_t input[8], char output[13])
{
    static const char alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int input_index = 0;
    int output_index = 0;

    while (input_index < 8)
    {
        int remaining = 8 - input_index;
        uint8_t a = input[input_index++];
        uint8_t b = remaining > 1 ? input[input_index++] : 0;
        uint8_t c = remaining > 2 ? input[input_index++] : 0;

        output[output_index++] = alphabet[(a >> 2) & 0x3f];
        output[output_index++] = alphabet[((a & 0x03) << 4) | ((b >> 4) & 0x0f)];
        output[output_index++] =
            remaining > 1 ? alphabet[((b & 0x0f) << 2) | ((c >> 6) & 0x03)] : '=';
        output[output_index++] = remaining > 2 ? alphabet[c & 0x3f] : '=';
    }

    output[12] = '\0';
}

#ifdef SELF_TEST
int main(void)
{
    const uint8_t account_id[8] = {0x78, 0x69, 0x5a, 0x4b, 0x3c, 0x2d, 0x1e, 0x8f};
    char encoded[13] = {0};

    base64_encode_account_id(account_id, encoded);

    assert(strcmp(encoded, "eGlaSzwtHo8=") == 0);
    assert(slot_key(1, REG_KEY_USER_ID_FIRST, REG_KEY_USER_ID_INVALID) == 125829376);
    assert(slot_key(16, REG_KEY_USER_ID_FIRST, REG_KEY_USER_ID_INVALID) == 126812416);
    assert(slot_key(0, REG_KEY_USER_ID_FIRST, REG_KEY_USER_ID_INVALID) == REG_KEY_USER_ID_INVALID);
    return 0;
}
#else
int main(void)
{
    int remoteplay_enabled = 0;
    int pairing_state = 0;
    int pairing_error = 0;
    uint64_t account_id = 0;
    uint32_t pin = 0;
    char account_id_base64[13] = {0};
    char message[256] = {0};
    time_t deadline;
    time_t last_log_time;

    log_message("Initializing Remote Play...");

    int result = sceRemoteplayInitialize(0, 0);
    if (result != 0)
    {
        snprintf(message, sizeof(message), "ERROR: sceRemoteplayInitialize failed: 0x%x", result);
        log_message(message);
        return -1;
    }

    if (sceRegMgrGetInt(REG_KEY_REMOTEPLAY_ENABLED, &remoteplay_enabled) != 0)
    {
        log_message("ERROR: Failed to read the Remote Play setting");
        return -1;
    }

    if (remoteplay_enabled != 1 && sceRegMgrSetInt(REG_KEY_REMOTEPLAY_ENABLED, 1) != 0)
    {
        log_message("ERROR: Failed to enable Remote Play");
        return -1;
    }

    int slot = current_user_slot();
    if (slot < 1)
    {
        log_message("ERROR: Failed to identify the foreground user");
        return -1;
    }

    int user_remoteplay_enabled = 0;
    int user_remoteplay_key =
        slot_key(slot, REG_KEY_USER_REMOTEPLAY_FIRST, REG_KEY_USER_REMOTEPLAY_INVALID);

    if (sceRegMgrGetInt(user_remoteplay_key, &user_remoteplay_enabled) == 0 &&
        user_remoteplay_enabled != 1 && sceRegMgrSetInt(user_remoteplay_key, 1) != 0)
    {
        log_message("ERROR: Failed to enable Remote Play for the foreground user");
        return -1;
    }

    int account_id_key = slot_key(slot, REG_KEY_ACCOUNT_ID_FIRST, REG_KEY_ACCOUNT_ID_INVALID);

    if (sceRegMgrGetBin(account_id_key, &account_id, sizeof(account_id)) != 0)
    {
        log_message("ERROR: Failed to read the foreground user's Account ID");
        return -1;
    }

    base64_encode_account_id((const uint8_t *)&account_id, account_id_base64);

    sceRemoteplayNotifyPinCodeError(1);

    result = sceRemoteplayGeneratePinCode(&pin);
    if (result != 0)
    {
        snprintf(message, sizeof(message), "ERROR: sceRemoteplayGeneratePinCode failed: 0x%x",
                 result);
        log_message(message);
        return -1;
    }

    snprintf(message, sizeof(message), "READY | PIN: %08u | Account ID: %s | Timeout: %ds", pin,
             account_id_base64, PAIRING_TIMEOUT_SECONDS);
    log_message(message);

    last_log_time = time(0);
    deadline = last_log_time + PAIRING_TIMEOUT_SECONDS;

    while (1)
    {
        time_t now = time(0);

        if (now >= deadline)
        {
            sceRemoteplayNotifyPinCodeError(1);
            log_message("Pairing expired");
            return 0;
        }

        pairing_state = 0;
        pairing_error = 0;
        result = sceRemoteplayConfirmDeviceRegist(&pairing_state, &pairing_error);

        if (result == 0 && pairing_state == 2)
        {
            log_message("Pairing completed successfully");
            return 0;
        }

        if (result == 0 && pairing_state == 3)
        {
            if (pairing_error == PAIR_ERROR_INVALID_PIN)
            {
                log_message("ERROR: Incorrect PIN");
            }
            else if (pairing_error == PAIR_ERROR_INVALID_ACCOUNT)
            {
                log_message("ERROR: Incorrect Account ID");
            }
            else
            {
                snprintf(message, sizeof(message), "ERROR: Pairing failed: 0x%x", pairing_error);
                log_message(message);
            }

            return -1;
        }

        if ((now - last_log_time) >= LOG_INTERVAL_SECONDS)
        {
            snprintf(message, sizeof(message),
                     "WAITING | PIN: %08u | Account ID: %s | Remaining: %lds", pin,
                     account_id_base64, (long)(deadline - now));
            log_message(message);
            last_log_time = now;
        }

        sleep(1);
    }
}
#endif
