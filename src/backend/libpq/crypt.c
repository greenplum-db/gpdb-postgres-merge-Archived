/*-------------------------------------------------------------------------
 *
 * crypt.c
 *	  Functions for dealing with encrypted passwords stored in
 *	  pg_authid.rolpassword.
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/backend/libpq/crypt.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include <unistd.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#include "catalog/pg_authid.h"
#include "common/md5.h"
#include "common/scram-common.h"
#include "libpq/crypt.h"
<<<<<<< HEAD
#include "libpq/md5.h"
#include "libpq/password_hash.h"
#include "libpq/pg_sha2.h"
=======
#include "libpq/scram.h"
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
#include "miscadmin.h"
#include "utils/builtins.h"
#include "utils/syscache.h"
#include "utils/timestamp.h"

bool
hash_password(const char *passwd, char *salt, size_t salt_len, char *buf)
{
	switch (password_hash_algorithm)
	{
		case PASSWORD_HASH_MD5:
			return pg_md5_encrypt(passwd, salt, salt_len, buf);
		case PASSWORD_HASH_SHA_256:
			return pg_sha256_encrypt(passwd, salt, salt_len, buf);
			break;
		default:
			elog(ERROR,
				 "unknown password hash algorithm number %d",
				 password_hash_algorithm);
	}
	return false; /* we never get here */
}


/*
 * Fetch stored password for a user, for authentication.
 *
 * On error, returns NULL, and stores a palloc'd string describing the reason,
 * for the postmaster log, in *logdetail.  The error reason should *not* be
 * sent to the client, to avoid giving away user information!
 */
<<<<<<< HEAD
int
hashed_passwd_verify(const Port *port, const char *role, char *client_pass,
				 char **logdetail)
=======
char *
get_role_password(const char *role, char **logdetail)
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
{
	TimestampTz vuntil = 0;
	HeapTuple	roleTup;
	Datum		datum;
	bool		isnull;
	char	   *shadow_pass;

	/* Get role info from pg_authid */
	roleTup = SearchSysCache1(AUTHNAME, PointerGetDatum(role));
	if (!HeapTupleIsValid(roleTup))
	{
		*logdetail = psprintf(_("Role \"%s\" does not exist."),
							  role);
		return NULL;			/* no such user */
	}

	datum = SysCacheGetAttr(AUTHNAME, roleTup,
							Anum_pg_authid_rolpassword, &isnull);
	if (isnull)
	{
		ReleaseSysCache(roleTup);
		*logdetail = psprintf(_("User \"%s\" has no password assigned."),
							  role);
		return NULL;			/* user has no password */
	}
	shadow_pass = TextDatumGetCString(datum);

	datum = SysCacheGetAttr(AUTHNAME, roleTup,
							Anum_pg_authid_rolvaliduntil, &isnull);
	if (!isnull)
		vuntil = DatumGetTimestampTz(datum);

	ReleaseSysCache(roleTup);

<<<<<<< HEAD
	CHECK_FOR_INTERRUPTS();

	/*
	 * Don't allow an empty password. Libpq treats an empty password the same
	 * as no password at all, and won't even try to authenticate. But other
	 * clients might, so allowing it would be confusing.
	 *
	 * For a plaintext password, we can simply check that it's not an empty
	 * string. For an encrypted password, check that it does not match the MD5
	 * hash of an empty string.
	 */
	if (*shadow_pass == '\0')
	{
		*logdetail = psprintf(_("User \"%s\" has an empty password."),
							  role);
		return STATUS_ERROR;	/* empty password */
	}
	if (isMD5(shadow_pass))
	{
		char		crypt_empty[MD5_PASSWD_LEN + 1];

		if (!pg_md5_encrypt("",
							port->user_name,
							strlen(port->user_name),
							crypt_empty))
			return STATUS_ERROR;
		if (strcmp(shadow_pass, crypt_empty) == 0)
		{
			*logdetail = psprintf(_("User \"%s\" has an empty password."),
								  role);
			return STATUS_ERROR;	/* empty password */
		}
	}

=======
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
	/*
	 * Password OK, but check to be sure we are not past rolvaliduntil
	 */
	if (!isnull && vuntil < GetCurrentTimestamp())
	{
<<<<<<< HEAD
		case uaMD5:
			crypt_pwd = palloc(MD5_PASSWD_LEN + 1);
			if (isMD5(shadow_pass))
			{
				/* stored password already encrypted, only do salt */
				if (!pg_md5_encrypt(shadow_pass + strlen("md5"),
									port->md5Salt,
									sizeof(port->md5Salt), crypt_pwd))
				{
					pfree(crypt_pwd);
					return STATUS_ERROR;
				}
			}
			else if (isSHA256(shadow_pass))
			{
				/* 
				 * Client supplied an MD5 hashed password but our password 
				 * is stored as SHA256 so we cannot compare the two.
				 */
				ereport(FATAL,
						(errcode(ERRCODE_INVALID_AUTHORIZATION_SPECIFICATION),
						 errmsg("MD5 authentication is not supported with "
								"SHA256 hashed passwords"),
						 errhint("Set an alternative authentication method "
								 "for this role in pg_hba.conf")));


			}
			else
			{
				/* stored password is plain, double-encrypt */
				char	   *crypt_pwd2 = palloc(MD5_PASSWD_LEN + 1);

				if (!pg_md5_encrypt(shadow_pass,
									port->user_name,
									strlen(port->user_name),
									crypt_pwd2))
				{
					pfree(crypt_pwd);
					pfree(crypt_pwd2);
					return STATUS_ERROR;
				}
				if (!pg_md5_encrypt(crypt_pwd2 + strlen("md5"),
									port->md5Salt,
									sizeof(port->md5Salt),
									crypt_pwd))
				{
					pfree(crypt_pwd);
					pfree(crypt_pwd2);
					return STATUS_ERROR;
				}
				pfree(crypt_pwd2);
			}
			break;
		default:
			if (isMD5(shadow_pass))
			{
				/* Encrypt user-supplied password to match stored MD5 */
				crypt_client_pass = palloc(MD5_PASSWD_LEN + 1);
				if (!pg_md5_encrypt(client_pass,
									port->user_name,
									strlen(port->user_name),
									crypt_client_pass))
				{
					pfree(crypt_client_pass);
					return STATUS_ERROR;
				}
			}
			else if (isSHA256(shadow_pass))
			{
				/* Encrypt user-supplied password to match the stored SHA-256 */
				crypt_client_pass = palloc(SHA256_PASSWD_LEN + 1);
				if (!pg_sha256_encrypt(client_pass,
									   port->user_name,
									   strlen(port->user_name),
									   crypt_client_pass))
				{
					pfree(crypt_client_pass);
					return STATUS_ERROR;
				}
			}
			crypt_pwd = shadow_pass;
			break;
=======
		*logdetail = psprintf(_("User \"%s\" has an expired password."),
							  role);
		return NULL;
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
	}

	return shadow_pass;
}

/*
 * What kind of a password verifier is 'shadow_pass'?
 */
PasswordType
get_password_type(const char *shadow_pass)
{
	char	   *encoded_salt;
	int			iterations;
	uint8		stored_key[SCRAM_KEY_LEN];
	uint8		server_key[SCRAM_KEY_LEN];

	if (strncmp(shadow_pass, "md5", 3) == 0 &&
		strlen(shadow_pass) == MD5_PASSWD_LEN &&
		strspn(shadow_pass + 3, MD5_PASSWD_CHARSET) == MD5_PASSWD_LEN - 3)
		return PASSWORD_TYPE_MD5;
	if (parse_scram_verifier(shadow_pass, &iterations, &encoded_salt,
							 stored_key, server_key))
		return PASSWORD_TYPE_SCRAM_SHA_256;
	return PASSWORD_TYPE_PLAINTEXT;
}

/*
 * Given a user-supplied password, convert it into a verifier of
 * 'target_type' kind.
 *
 * If the password is already in encrypted form, we cannot reverse the
 * hash, so it is stored as it is regardless of the requested type.
 */
char *
encrypt_password(PasswordType target_type, const char *role,
				 const char *password)
{
	PasswordType guessed_type = get_password_type(password);
	char	   *encrypted_password;

	if (guessed_type != PASSWORD_TYPE_PLAINTEXT)
	{
		/*
		 * Cannot convert an already-encrypted password from one format to
		 * another, so return it as it is.
		 */
		return pstrdup(password);
	}

	switch (target_type)
	{
		case PASSWORD_TYPE_MD5:
			encrypted_password = palloc(MD5_PASSWD_LEN + 1);

			if (!pg_md5_encrypt(password, role, strlen(role),
								encrypted_password))
				elog(ERROR, "password encryption failed");
			return encrypted_password;

		case PASSWORD_TYPE_SCRAM_SHA_256:
			return pg_be_scram_build_verifier(password);

		case PASSWORD_TYPE_PLAINTEXT:
			elog(ERROR, "cannot encrypt password with 'plaintext'");
	}

	/*
	 * This shouldn't happen, because the above switch statements should
	 * handle every combination of source and target password types.
	 */
	elog(ERROR, "cannot encrypt password to requested type");
	return NULL;				/* keep compiler quiet */
}

/*
 * Check MD5 authentication response, and return STATUS_OK or STATUS_ERROR.
 *
 * 'shadow_pass' is the user's correct password or password hash, as stored
 * in pg_authid.rolpassword.
 * 'client_pass' is the response given by the remote user to the MD5 challenge.
 * 'md5_salt' is the salt used in the MD5 authentication challenge.
 *
 * In the error case, optionally store a palloc'd string at *logdetail
 * that will be sent to the postmaster log (but not the client).
 */
int
md5_crypt_verify(const char *role, const char *shadow_pass,
				 const char *client_pass,
				 const char *md5_salt, int md5_salt_len,
				 char **logdetail)
{
	int			retval;
	char		crypt_pwd[MD5_PASSWD_LEN + 1];

	Assert(md5_salt_len > 0);

	if (get_password_type(shadow_pass) != PASSWORD_TYPE_MD5)
	{
		/* incompatible password hash format. */
		*logdetail = psprintf(_("User \"%s\" has a password that cannot be used with MD5 authentication."),
							  role);
		return STATUS_ERROR;
	}

	/*
	 * Compute the correct answer for the MD5 challenge.
	 *
	 * We do not bother setting logdetail for any pg_md5_encrypt failure
	 * below: the only possible error is out-of-memory, which is unlikely, and
	 * if it did happen adding a psprintf call would only make things worse.
	 */
	/* stored password already encrypted, only do salt */
	if (!pg_md5_encrypt(shadow_pass + strlen("md5"),
						md5_salt, md5_salt_len,
						crypt_pwd))
	{
		return STATUS_ERROR;
	}

	if (strcmp(client_pass, crypt_pwd) == 0)
		retval = STATUS_OK;
	else
	{
		*logdetail = psprintf(_("Password does not match for user \"%s\"."),
							  role);
		retval = STATUS_ERROR;
	}

	return retval;
}

/*
 * Check given password for given user, and return STATUS_OK or STATUS_ERROR.
 *
 * 'shadow_pass' is the user's correct password hash, as stored in
 * pg_authid.rolpassword.
 * 'client_pass' is the password given by the remote user.
 *
 * In the error case, optionally store a palloc'd string at *logdetail
 * that will be sent to the postmaster log (but not the client).
 */
int
plain_crypt_verify(const char *role, const char *shadow_pass,
				   const char *client_pass,
				   char **logdetail)
{
	char		crypt_client_pass[MD5_PASSWD_LEN + 1];

	/*
	 * Client sent password in plaintext.  If we have an MD5 hash stored, hash
	 * the password the client sent, and compare the hashes.  Otherwise
	 * compare the plaintext passwords directly.
	 */
	switch (get_password_type(shadow_pass))
	{
		case PASSWORD_TYPE_SCRAM_SHA_256:
			if (scram_verify_plain_password(role,
											client_pass,
											shadow_pass))
			{
				return STATUS_OK;
			}
			else
			{
				*logdetail = psprintf(_("Password does not match for user \"%s\"."),
									  role);
				return STATUS_ERROR;
			}
			break;

		case PASSWORD_TYPE_MD5:
			if (!pg_md5_encrypt(client_pass,
								role,
								strlen(role),
								crypt_client_pass))
			{
				/*
				 * We do not bother setting logdetail for pg_md5_encrypt
				 * failure: the only possible error is out-of-memory, which is
				 * unlikely, and if it did happen adding a psprintf call would
				 * only make things worse.
				 */
				return STATUS_ERROR;
			}
			if (strcmp(crypt_client_pass, shadow_pass) == 0)
				return STATUS_OK;
			else
			{
				*logdetail = psprintf(_("Password does not match for user \"%s\"."),
									  role);
				return STATUS_ERROR;
			}
			break;

		case PASSWORD_TYPE_PLAINTEXT:

			/*
			 * We never store passwords in plaintext, so this shouldn't
			 * happen.
			 */
			break;
	}

	/*
	 * This shouldn't happen.  Plain "password" authentication is possible
	 * with any kind of stored password hash.
	 */
	*logdetail = psprintf(_("Password of user \"%s\" is in unrecognized format."),
						  role);
	return STATUS_ERROR;
}
