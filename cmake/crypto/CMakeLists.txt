cmake_minimum_required(VERSION 2.6)

project(crypto)

set(SOURCE_DIR ${ROOT_DIR}/3rdparty/openssl/crypto)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR} 
	${SOURCE_DIR}/../../openssl-win32
	${SOURCE_DIR}/../../openssl-win32/openssl
)

add_definitions(
    -D_CRT_SECURE_NO_DEPRECATE
    -DDSO_WIN32
    -DOPENSSL_SYSNAME_WIN32
    -DWIN32_LEAN_AND_MEAN
    -DL_ENDIAN
    -DOPENSSL_NO_ASM
    -DOPENSSL_NO_INLINE_ASM
    -DNO_WINDOWS_BRAINDEATH
    -DMK1MF_BUILD
    -DMK1MF_PLATFORM_VC_WIN32
	-D_LIB
)
 
 
# _CRT_SECURE_NO_WARNINGS;
#  MK1MF_BUILD;MK1MF_PLATFORM_VC_WIN32;WIN32;NDEBUG;_LIB;%(PreprocessorDefinitions)
 
set(crypto_SRCS
    ${SOURCE_DIR}/asn1/a_bitstr.c
    ${SOURCE_DIR}/asn1/a_bool.c
    ${SOURCE_DIR}/asn1/a_bytes.c
    ${SOURCE_DIR}/asn1/a_d2i_fp.c
    ${SOURCE_DIR}/asn1/a_digest.c
    ${SOURCE_DIR}/asn1/a_dup.c
    ${SOURCE_DIR}/asn1/a_enum.c
    ${SOURCE_DIR}/asn1/a_gentm.c
    ${SOURCE_DIR}/asn1/a_i2d_fp.c
    ${SOURCE_DIR}/asn1/a_int.c
    ${SOURCE_DIR}/asn1/a_mbstr.c
    ${SOURCE_DIR}/asn1/a_object.c
    ${SOURCE_DIR}/asn1/a_octet.c
    ${SOURCE_DIR}/asn1/a_print.c
    ${SOURCE_DIR}/asn1/a_set.c
    ${SOURCE_DIR}/asn1/a_sign.c
    ${SOURCE_DIR}/asn1/a_strex.c
    ${SOURCE_DIR}/asn1/a_strnid.c
    ${SOURCE_DIR}/asn1/a_time.c
    ${SOURCE_DIR}/asn1/a_type.c
    ${SOURCE_DIR}/asn1/a_utctm.c
    ${SOURCE_DIR}/asn1/a_utf8.c
    ${SOURCE_DIR}/asn1/a_verify.c
    ${SOURCE_DIR}/asn1/ameth_lib.c
    ${SOURCE_DIR}/asn1/asn1_err.c
    ${SOURCE_DIR}/asn1/asn1_gen.c
    ${SOURCE_DIR}/asn1/asn1_lib.c
    ${SOURCE_DIR}/asn1/asn1_par.c
    ${SOURCE_DIR}/asn1/asn_mime.c
    ${SOURCE_DIR}/asn1/asn_moid.c
    ${SOURCE_DIR}/asn1/asn_pack.c
    ${SOURCE_DIR}/asn1/bio_asn1.c
    ${SOURCE_DIR}/asn1/bio_ndef.c
    ${SOURCE_DIR}/asn1/d2i_pr.c
    ${SOURCE_DIR}/asn1/d2i_pu.c
    ${SOURCE_DIR}/asn1/evp_asn1.c
    ${SOURCE_DIR}/asn1/f_enum.c
    ${SOURCE_DIR}/asn1/f_int.c
    ${SOURCE_DIR}/asn1/f_string.c
    ${SOURCE_DIR}/asn1/i2d_pr.c
    ${SOURCE_DIR}/asn1/i2d_pu.c
    ${SOURCE_DIR}/asn1/nsseq.c
    ${SOURCE_DIR}/asn1/p5_pbe.c
    ${SOURCE_DIR}/asn1/p5_pbev2.c
    ${SOURCE_DIR}/asn1/p8_pkey.c
    ${SOURCE_DIR}/asn1/t_crl.c
    ${SOURCE_DIR}/asn1/t_pkey.c
    ${SOURCE_DIR}/asn1/t_req.c
    ${SOURCE_DIR}/asn1/t_spki.c
    ${SOURCE_DIR}/asn1/t_x509.c
    ${SOURCE_DIR}/asn1/t_x509a.c
    ${SOURCE_DIR}/asn1/tasn_dec.c
    ${SOURCE_DIR}/asn1/tasn_enc.c
    ${SOURCE_DIR}/asn1/tasn_fre.c
    ${SOURCE_DIR}/asn1/tasn_new.c
    ${SOURCE_DIR}/asn1/tasn_prn.c
    ${SOURCE_DIR}/asn1/tasn_typ.c
    ${SOURCE_DIR}/asn1/tasn_utl.c
    ${SOURCE_DIR}/asn1/x_algor.c
    ${SOURCE_DIR}/asn1/x_attrib.c
    ${SOURCE_DIR}/asn1/x_bignum.c
    ${SOURCE_DIR}/asn1/x_crl.c
    ${SOURCE_DIR}/asn1/x_exten.c
    ${SOURCE_DIR}/asn1/x_info.c
    ${SOURCE_DIR}/asn1/x_long.c
    ${SOURCE_DIR}/asn1/x_name.c
    ${SOURCE_DIR}/asn1/x_nx509.c
    ${SOURCE_DIR}/asn1/x_pkey.c
    ${SOURCE_DIR}/asn1/x_pubkey.c
    ${SOURCE_DIR}/asn1/x_req.c
    ${SOURCE_DIR}/asn1/x_sig.c
    ${SOURCE_DIR}/asn1/x_spki.c
    ${SOURCE_DIR}/asn1/x_val.c
    ${SOURCE_DIR}/asn1/x_x509.c
    ${SOURCE_DIR}/asn1/x_x509a.c

    ${SOURCE_DIR}/bio/b_dump.c
    ${SOURCE_DIR}/bio/b_print.c
    ${SOURCE_DIR}/bio/b_sock.c
	${SOURCE_DIR}/bio/bf_buff.c
    ${SOURCE_DIR}/bio/bio_cb.c
    ${SOURCE_DIR}/bio/bio_err.c
    ${SOURCE_DIR}/bio/bio_lib.c
    ${SOURCE_DIR}/bio/bss_acpt.c
    ${SOURCE_DIR}/bio/bss_bio.c
    ${SOURCE_DIR}/bio/bss_conn.c
    ${SOURCE_DIR}/bio/bss_fd.c
    ${SOURCE_DIR}/bio/bss_file.c
    ${SOURCE_DIR}/bio/bss_mem.c
    ${SOURCE_DIR}/bio/bss_null.c
    ${SOURCE_DIR}/bio/bss_sock.c

    ${SOURCE_DIR}/bn/bn_add.c
    ${SOURCE_DIR}/bn/bn_asm.c
    ${SOURCE_DIR}/bn/bn_blind.c
    ${SOURCE_DIR}/bn/bn_const.c
    ${SOURCE_DIR}/bn/bn_ctx.c
    ${SOURCE_DIR}/bn/bn_div.c
    ${SOURCE_DIR}/bn/bn_err.c
    ${SOURCE_DIR}/bn/bn_exp.c
    ${SOURCE_DIR}/bn/bn_exp2.c
    ${SOURCE_DIR}/bn/bn_gcd.c
    ${SOURCE_DIR}/bn/bn_kron.c
    ${SOURCE_DIR}/bn/bn_lib.c
    ${SOURCE_DIR}/bn/bn_mod.c
    ${SOURCE_DIR}/bn/bn_mont.c
    ${SOURCE_DIR}/bn/bn_mpi.c
    ${SOURCE_DIR}/bn/bn_mul.c
    ${SOURCE_DIR}/bn/bn_nist.c
    ${SOURCE_DIR}/bn/bn_prime.c
    ${SOURCE_DIR}/bn/bn_print.c
    ${SOURCE_DIR}/bn/bn_rand.c
    ${SOURCE_DIR}/bn/bn_recp.c
    ${SOURCE_DIR}/bn/bn_shift.c
    ${SOURCE_DIR}/bn/bn_sqr.c
    ${SOURCE_DIR}/bn/bn_sqrt.c
    ${SOURCE_DIR}/bn/bn_word.c

    ${SOURCE_DIR}/buffer/buf_err.c
    ${SOURCE_DIR}/buffer/buf_str.c
    ${SOURCE_DIR}/buffer/buffer.c

    ${SOURCE_DIR}/cmac/cm_ameth.c
    ${SOURCE_DIR}/cmac/cm_pmeth.c
    ${SOURCE_DIR}/cmac/cmac.c

    ${SOURCE_DIR}/conf/conf_api.c
    ${SOURCE_DIR}/conf/conf_def.c
    ${SOURCE_DIR}/conf/conf_err.c
    ${SOURCE_DIR}/conf/conf_lib.c
    ${SOURCE_DIR}/conf/conf_mall.c
    ${SOURCE_DIR}/conf/conf_mod.c
    ${SOURCE_DIR}/conf/conf_sap.c

    ${SOURCE_DIR}/cpt_err.c
    ${SOURCE_DIR}/cryptlib.c
    ${SOURCE_DIR}/cversion.c
    ${SOURCE_DIR}/ebcdic.c
    ${SOURCE_DIR}/ex_data.c
    ${SOURCE_DIR}/getenv.c
    ${SOURCE_DIR}/mem.c
    ${SOURCE_DIR}/mem_clr.c
    ${SOURCE_DIR}/mem_dbg.c
    ${SOURCE_DIR}/o_dir.c
    ${SOURCE_DIR}/o_init.c
    ${SOURCE_DIR}/o_str.c
    ${SOURCE_DIR}/o_time.c
    ${SOURCE_DIR}/objects/o_names.c
    ${SOURCE_DIR}/uid.c

    ${SOURCE_DIR}/dsa/dsa_ameth.c
    ${SOURCE_DIR}/dsa/dsa_asn1.c
    ${SOURCE_DIR}/dsa/dsa_err.c
    ${SOURCE_DIR}/dsa/dsa_gen.c
    ${SOURCE_DIR}/dsa/dsa_key.c
    ${SOURCE_DIR}/dsa/dsa_lib.c
    ${SOURCE_DIR}/dsa/dsa_ossl.c
    ${SOURCE_DIR}/dsa/dsa_pmeth.c
    ${SOURCE_DIR}/dsa/dsa_prn.c
    ${SOURCE_DIR}/dsa/dsa_sign.c
    ${SOURCE_DIR}/dsa/dsa_vrf.c

    ${SOURCE_DIR}/dso/dso_beos.c
    ${SOURCE_DIR}/dso/dso_dl.c
    ${SOURCE_DIR}/dso/dso_dlfcn.c
    ${SOURCE_DIR}/dso/dso_err.c
    ${SOURCE_DIR}/dso/dso_lib.c
    ${SOURCE_DIR}/dso/dso_null.c
    ${SOURCE_DIR}/dso/dso_openssl.c
    ${SOURCE_DIR}/dso/dso_vms.c
    ${SOURCE_DIR}/dso/dso_win32.c

    ${SOURCE_DIR}/err/err.c
    ${SOURCE_DIR}/err/err_all.c
    ${SOURCE_DIR}/err/err_prn.c

    ${SOURCE_DIR}/evp/bio_b64.c
    ${SOURCE_DIR}/evp/bio_enc.c
    ${SOURCE_DIR}/evp/bio_md.c
    ${SOURCE_DIR}/evp/bio_ok.c
	${SOURCE_DIR}/evp/digest.c
    ${SOURCE_DIR}/evp/e_null.c
    ${SOURCE_DIR}/evp/e_rc4.c
    ${SOURCE_DIR}/evp/e_rc4_hmac_md5.c
    ${SOURCE_DIR}/evp/e_seed.c
    ${SOURCE_DIR}/evp/encode.c
    ${SOURCE_DIR}/evp/evp_cnf.c
    ${SOURCE_DIR}/evp/evp_enc.c
    ${SOURCE_DIR}/evp/evp_err.c
    ${SOURCE_DIR}/evp/evp_key.c
    ${SOURCE_DIR}/evp/evp_lib.c
    ${SOURCE_DIR}/evp/evp_pbe.c
    ${SOURCE_DIR}/evp/evp_pkey.c
    ${SOURCE_DIR}/evp/m_dss.c
    ${SOURCE_DIR}/evp/m_dss1.c
    ${SOURCE_DIR}/evp/m_ecdsa.c
    ${SOURCE_DIR}/evp/m_md4.c
    ${SOURCE_DIR}/evp/m_md5.c
    ${SOURCE_DIR}/evp/m_null.c
    ${SOURCE_DIR}/evp/m_sha.c
    ${SOURCE_DIR}/evp/m_sha1.c
    ${SOURCE_DIR}/evp/m_sigver.c
    ${SOURCE_DIR}/evp/names.c
    ${SOURCE_DIR}/evp/p5_crpt.c
    ${SOURCE_DIR}/evp/p5_crpt2.c
    ${SOURCE_DIR}/evp/p_dec.c
    ${SOURCE_DIR}/evp/p_enc.c
    ${SOURCE_DIR}/evp/p_lib.c
    ${SOURCE_DIR}/evp/p_seal.c
    ${SOURCE_DIR}/evp/p_sign.c
    ${SOURCE_DIR}/evp/p_verify.c
    ${SOURCE_DIR}/evp/pmeth_fn.c
    ${SOURCE_DIR}/evp/pmeth_gn.c
    ${SOURCE_DIR}/evp/pmeth_lib.c

    ${SOURCE_DIR}/hmac/hm_ameth.c
    ${SOURCE_DIR}/hmac/hm_pmeth.c
    ${SOURCE_DIR}/hmac/hmac.c

    ${SOURCE_DIR}/krb5/krb5_asn.c

    ${SOURCE_DIR}/lhash/lh_stats.c
    ${SOURCE_DIR}/lhash/lhash.c

    ${SOURCE_DIR}/md4/md4_dgst.c
    ${SOURCE_DIR}/md4/md4_one.c

    ${SOURCE_DIR}/md5/md5_dgst.c
    ${SOURCE_DIR}/md5/md5_one.c

    ${SOURCE_DIR}/modes/cbc128.c
    ${SOURCE_DIR}/modes/ccm128.c
    ${SOURCE_DIR}/modes/cfb128.c
    ${SOURCE_DIR}/modes/ctr128.c
    ${SOURCE_DIR}/modes/cts128.c
    ${SOURCE_DIR}/modes/gcm128.c
    ${SOURCE_DIR}/modes/ofb128.c
    ${SOURCE_DIR}/modes/wrap128.c
    ${SOURCE_DIR}/modes/xts128.c

    ${SOURCE_DIR}/objects/obj_dat.c
    ${SOURCE_DIR}/objects/obj_err.c
    ${SOURCE_DIR}/objects/obj_lib.c
    ${SOURCE_DIR}/objects/obj_xref.c

    ${SOURCE_DIR}/ocsp/ocsp_asn.c
    ${SOURCE_DIR}/ocsp/ocsp_cl.c
    ${SOURCE_DIR}/ocsp/ocsp_err.c
    ${SOURCE_DIR}/ocsp/ocsp_ext.c
    ${SOURCE_DIR}/ocsp/ocsp_ht.c
    ${SOURCE_DIR}/ocsp/ocsp_lib.c
    ${SOURCE_DIR}/ocsp/ocsp_prn.c
    ${SOURCE_DIR}/ocsp/ocsp_srv.c
    ${SOURCE_DIR}/ocsp/ocsp_vfy.c

    ${SOURCE_DIR}/pem/pem_all.c
    ${SOURCE_DIR}/pem/pem_err.c
    ${SOURCE_DIR}/pem/pem_info.c
    ${SOURCE_DIR}/pem/pem_lib.c
    ${SOURCE_DIR}/pem/pem_oth.c
    ${SOURCE_DIR}/pem/pem_pk8.c
    ${SOURCE_DIR}/pem/pem_pkey.c
    ${SOURCE_DIR}/pem/pem_seal.c
    ${SOURCE_DIR}/pem/pem_sign.c
    ${SOURCE_DIR}/pem/pem_x509.c
    ${SOURCE_DIR}/pem/pem_xaux.c

    ${SOURCE_DIR}/pkcs12/p12_add.c
    ${SOURCE_DIR}/pkcs12/p12_asn.c
    ${SOURCE_DIR}/pkcs12/p12_attr.c
    ${SOURCE_DIR}/pkcs12/p12_crpt.c
    ${SOURCE_DIR}/pkcs12/p12_crt.c
    ${SOURCE_DIR}/pkcs12/p12_decr.c
    ${SOURCE_DIR}/pkcs12/p12_init.c
    ${SOURCE_DIR}/pkcs12/p12_key.c
    ${SOURCE_DIR}/pkcs12/p12_kiss.c
    ${SOURCE_DIR}/pkcs12/p12_mutl.c
    ${SOURCE_DIR}/pkcs12/p12_npas.c
    ${SOURCE_DIR}/pkcs12/p12_p8d.c
    ${SOURCE_DIR}/pkcs12/p12_p8e.c
    ${SOURCE_DIR}/pkcs12/p12_utl.c
    ${SOURCE_DIR}/pkcs12/pk12err.c

    ${SOURCE_DIR}/pkcs7/bio_pk7.c
    ${SOURCE_DIR}/pkcs7/pk7_asn1.c
    ${SOURCE_DIR}/pkcs7/pk7_attr.c
    ${SOURCE_DIR}/pkcs7/pk7_doit.c
    ${SOURCE_DIR}/pkcs7/pk7_lib.c
    ${SOURCE_DIR}/pkcs7/pk7_mime.c
    ${SOURCE_DIR}/pkcs7/pk7_smime.c
    ${SOURCE_DIR}/pkcs7/pkcs7err.c

    ${SOURCE_DIR}/pqueue/pqueue.c

    ${SOURCE_DIR}/rand/md_rand.c
    ${SOURCE_DIR}/rand/rand_egd.c
    ${SOURCE_DIR}/rand/rand_err.c
    ${SOURCE_DIR}/rand/rand_lib.c
    ${SOURCE_DIR}/rand/rand_win.c
    ${SOURCE_DIR}/rand/randfile.c

    ${SOURCE_DIR}/rc4/rc4_enc.c
    ${SOURCE_DIR}/rc4/rc4_skey.c
    ${SOURCE_DIR}/rc4/rc4_utl.c

    ${SOURCE_DIR}/sha/sha1_one.c
    ${SOURCE_DIR}/sha/sha1dgst.c
    ${SOURCE_DIR}/sha/sha256.c
    ${SOURCE_DIR}/sha/sha512.c
    ${SOURCE_DIR}/sha/sha_dgst.c
    ${SOURCE_DIR}/sha/sha_one.c

    ${SOURCE_DIR}/stack/stack.c

 #  ${SOURCE_DIR}/threads/th-lock.c

    ${SOURCE_DIR}/ts/ts_asn1.c
    ${SOURCE_DIR}/ts/ts_conf.c
    ${SOURCE_DIR}/ts/ts_err.c
    ${SOURCE_DIR}/ts/ts_lib.c
    ${SOURCE_DIR}/ts/ts_req_print.c
    ${SOURCE_DIR}/ts/ts_req_utils.c
    ${SOURCE_DIR}/ts/ts_rsp_print.c
    ${SOURCE_DIR}/ts/ts_rsp_sign.c
    ${SOURCE_DIR}/ts/ts_rsp_utils.c
    ${SOURCE_DIR}/ts/ts_rsp_verify.c
    ${SOURCE_DIR}/ts/ts_verify_ctx.c

    ${SOURCE_DIR}/txt_db/txt_db.c

    ${SOURCE_DIR}/ui/ui_compat.c
    ${SOURCE_DIR}/ui/ui_err.c
    ${SOURCE_DIR}/ui/ui_lib.c
    ${SOURCE_DIR}/ui/ui_openssl.c
    ${SOURCE_DIR}/ui/ui_util.c

    ${SOURCE_DIR}/x509/by_dir.c
    ${SOURCE_DIR}/x509/by_file.c
    ${SOURCE_DIR}/x509/x509_att.c
    ${SOURCE_DIR}/x509/x509_cmp.c
    ${SOURCE_DIR}/x509/x509_d2.c
    ${SOURCE_DIR}/x509/x509_def.c
    ${SOURCE_DIR}/x509/x509_err.c
    ${SOURCE_DIR}/x509/x509_ext.c
    ${SOURCE_DIR}/x509/x509_lu.c
    ${SOURCE_DIR}/x509/x509_obj.c
    ${SOURCE_DIR}/x509/x509_r2x.c
    ${SOURCE_DIR}/x509/x509_req.c
    ${SOURCE_DIR}/x509/x509_set.c
    ${SOURCE_DIR}/x509/x509_trs.c
    ${SOURCE_DIR}/x509/x509_txt.c
    ${SOURCE_DIR}/x509/x509_v3.c
    ${SOURCE_DIR}/x509/x509_vfy.c
    ${SOURCE_DIR}/x509/x509_vpm.c
    ${SOURCE_DIR}/x509/x509cset.c
    ${SOURCE_DIR}/x509/x509name.c
    ${SOURCE_DIR}/x509/x509rset.c
    ${SOURCE_DIR}/x509/x509spki.c
    ${SOURCE_DIR}/x509/x509type.c
    ${SOURCE_DIR}/x509/x_all.c
    ${SOURCE_DIR}/x509v3/pcy_cache.c
    ${SOURCE_DIR}/x509v3/pcy_data.c
    ${SOURCE_DIR}/x509v3/pcy_lib.c
    ${SOURCE_DIR}/x509v3/pcy_map.c
    ${SOURCE_DIR}/x509v3/pcy_node.c
    ${SOURCE_DIR}/x509v3/pcy_tree.c
    ${SOURCE_DIR}/x509v3/v3_akey.c
    ${SOURCE_DIR}/x509v3/v3_akeya.c
    ${SOURCE_DIR}/x509v3/v3_alt.c
    ${SOURCE_DIR}/x509v3/v3_bcons.c
    ${SOURCE_DIR}/x509v3/v3_bitst.c
    ${SOURCE_DIR}/x509v3/v3_conf.c
    ${SOURCE_DIR}/x509v3/v3_cpols.c
    ${SOURCE_DIR}/x509v3/v3_crld.c
    ${SOURCE_DIR}/x509v3/v3_enum.c
    ${SOURCE_DIR}/x509v3/v3_extku.c
    ${SOURCE_DIR}/x509v3/v3_genn.c
    ${SOURCE_DIR}/x509v3/v3_ia5.c
    ${SOURCE_DIR}/x509v3/v3_info.c
    ${SOURCE_DIR}/x509v3/v3_int.c
    ${SOURCE_DIR}/x509v3/v3_lib.c
    ${SOURCE_DIR}/x509v3/v3_ncons.c
    ${SOURCE_DIR}/x509v3/v3_ocsp.c
    ${SOURCE_DIR}/x509v3/v3_pci.c
    ${SOURCE_DIR}/x509v3/v3_pcia.c
    ${SOURCE_DIR}/x509v3/v3_pcons.c
    ${SOURCE_DIR}/x509v3/v3_pku.c
    ${SOURCE_DIR}/x509v3/v3_pmaps.c
    ${SOURCE_DIR}/x509v3/v3_prn.c
    ${SOURCE_DIR}/x509v3/v3_purp.c
    ${SOURCE_DIR}/x509v3/v3_scts.c
    ${SOURCE_DIR}/x509v3/v3_skey.c
    ${SOURCE_DIR}/x509v3/v3_sxnet.c
    ${SOURCE_DIR}/x509v3/v3_utl.c
    ${SOURCE_DIR}/x509v3/v3err.c
)

add_library(crypto OBJECT ${crypto_SRCS})
