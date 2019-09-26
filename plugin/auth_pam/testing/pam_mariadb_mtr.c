/*
  This code is in the public domain and has no copyright.

  Pam module to test pam authentication plugin. Used in pam tests.
  Linux only.
  
  Install as appropriate (for example, in /lib/security/).
  see also mariadb_mtr.conf
*/

#include <stdlib.h>
#include <string.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

#define N 3

#if defined(SOLARIS) || defined(__sun)
#define PAM_CONST
#else
#define PAM_CONST const
#endif

int pam_sm_authenticate(pam_handle_t *pamh, int flags,
                        int argc, const char *argv[])
{
  struct pam_conv *conv;
  struct pam_response *resp = 0;
  int pam_err, retval = PAM_SYSTEM_ERR;
  struct pam_message msg[N] = {
    { PAM_TEXT_INFO, "Challenge input first." },
    { PAM_PROMPT_ECHO_OFF, "Enter:" },
    { PAM_ERROR_MSG, "Now, the magic number!" }
  };
  PAM_CONST struct pam_message *msgp[N] = { msg, msg+1, msg+2 };
  char *r1 = 0, *r2 = 0;

  pam_err = pam_get_item(pamh, PAM_CONV, (PAM_CONST void **)&conv);
  if (pam_err != PAM_SUCCESS)
    goto ret;

  pam_err = (*conv->conv)(N, msgp, &resp, conv->appdata_ptr);

  if (pam_err != PAM_SUCCESS || !resp || !((r1= resp[1].resp)))
    goto ret;

  if (strcmp(r1, "cleartext good") == 0)
    retval = PAM_SUCCESS;
  else if (strcmp(r1, "cleartext bad") == 0)
    retval = PAM_AUTH_ERR;
  else
  {
    free(resp);
    msg[0].msg_style = PAM_PROMPT_ECHO_ON;
    msg[0].msg = "PIN:";
    pam_err = (*conv->conv)(1, msgp, &resp, conv->appdata_ptr);

    if (pam_err != PAM_SUCCESS || !resp || !((r2= resp[0].resp)))
      goto ret;

    /* Produce the crash for testing purposes. */
    if (strcmp(r1, "crash pam module") == 0 && atoi(r2) == 616)
      abort();

    if (strlen(r1) == (size_t)atoi(r2) % 100)
      retval = PAM_SUCCESS;
    else
      retval = PAM_AUTH_ERR;
  }

  if (argc > 0 && argv[0])
    pam_set_item(pamh, PAM_USER, argv[0]);

ret:
  free(resp);
  free(r1);
  free(r2);
  return retval;
}

int pam_sm_setcred(pam_handle_t *pamh, int flags,
                   int argc, const char *argv[])
{

    return PAM_SUCCESS;
}

