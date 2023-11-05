static int CVE_2010_3880_PATCHED_inet_diag_dump_reqs(struct sk_buff *skb, struct sock *sk,
			       struct netlink_callback *cb)
{
	struct inet_diag_entry entry;
	struct inet_diag_req *r = NLMSG_DATA(cb->nlh);
	struct inet_connection_sock *icsk = inet_csk(sk);
	struct listen_sock *lopt;
	const struct nlattr *bc = NULL;
	struct inet_sock *inet = inet_sk(sk);
	int j, s_j;
	int reqnum, s_reqnum;
	int err = 0;

	s_j = cb->args[3];
	s_reqnum = cb->args[4];

	if (s_j > 0)
		s_j--;

	entry.family = sk->sk_family;

	read_lock_bh(&icsk->icsk_accept_queue.syn_wait_lock);

	lopt = icsk->icsk_accept_queue.listen_opt;
	if (!lopt || !lopt->qlen)
		goto out;

	if (nlmsg_attrlen(cb->nlh, sizeof(*r))) {
		bc = nlmsg_find_attr(cb->nlh, sizeof(*r),
				     INET_DIAG_REQ_BYTECODE);
		entry.sport = inet->inet_num;
		entry.userlocks = sk->sk_userlocks;
	}

	for (j = s_j; j < lopt->nr_table_entries; j++) {
		struct request_sock *req, *head = lopt->syn_table[j];

		reqnum = 0;
		for (req = head; req; reqnum++, req = req->dl_next) {
			struct inet_request_sock *ireq = inet_rsk(req);

			if (reqnum < s_reqnum)
				continue;
			if (r->id.idiag_dport != ireq->rmt_port &&
			    r->id.idiag_dport)
				continue;

			if (bc) {
				entry.saddr =
#if defined(CONFIG_IPV6) || defined (CONFIG_IPV6_MODULE)
					(entry.family == AF_INET6) ?
					inet6_rsk(req)->loc_addr.s6_addr32 :
#endif
					&ireq->loc_addr;
				entry.daddr =
#if defined(CONFIG_IPV6) || defined (CONFIG_IPV6_MODULE)
					(entry.family == AF_INET6) ?
					inet6_rsk(req)->rmt_addr.s6_addr32 :
#endif
					&ireq->rmt_addr;
				entry.dport = ntohs(ireq->rmt_port);

				if (!inet_diag_bc_run(nla_data(bc),
						      nla_len(bc), &entry))
					continue;
			}

			err = inet_diag_fill_req(skb, sk, req,
					       NETLINK_CB(cb->skb).pid,
					       cb->nlh->nlmsg_seq, cb->nlh);
			if (err < 0) {
				cb->args[3] = j + 1;
				cb->args[4] = reqnum;
				goto out;
			}
		}

		s_reqnum = 0;
	}

out:
	read_unlock_bh(&icsk->icsk_accept_queue.syn_wait_lock);

	return err;
}
