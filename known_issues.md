# Known Issues

> The first two entries below are marked as **EXAMPLE** to demonstrate the formatting and level of detail expected for real issues. Replace or remove them when adding actual project issues.

## Example Issues (Formatting Reference)

### Ticket 0002 (EXAMPLE – Placeholder)
- Status: Pending (Not Resolved)
- Summary: External API call to McDonalds ordering endpoint intermittently hangs; receives HTTP 100 (continue) then no final response.
- Description: When attempting to place a "Big Mac" order via custom `x401` protocol abstraction the client stalls after an initial 100 Continue. Subsequent retries over VPN produce the same incomplete transaction, suggesting a networking or proxy handshake issue rather than application logic.
- Environment: Windows 10, VPN enabled, requests-based client v1.2.3
- Date Reported: 2021-10-11
- Date Fixed: —
- Suspected Root Cause: VPN MTU fragmentation or missing final ACK from upstream gateway.
- Next Action: Capture packet trace (Wireshark) off-VPN; verify server expects additional headers; test without compression.
- Proposed Fix (Placeholder): Adjust client to disable HTTP/1.1 Expect header or force full payload flush.
- Learnings (Pending): Ensure network path validation before attributing failures to application layer.

### Ticket 0001 (EXAMPLE – Outdated Dependency)
- Status: Resolved ✅
- Summary: API call to McDonalds endpoint fails with HTTP 401 Unauthorized due to outdated request library sending deprecated auth header.
- Description: Ordering request using `x401` protocol wrapper returned 401. Investigation showed the underlying HTTP client library was an outdated version that still emitted a legacy auth token format. Upgrading the dependency corrected header construction and authentication succeeded.
- Environment: Windows 10, requests (old) v2.x (outdated), no VPN.
- Date Reported: 2021-09-11
- Date Fixed: 2021-10-12
- Fix Commit: `<INSERT COMMIT SHA>` (Upgrade dependency; regenerate lock file; adjust auth header normalization.)
- Root Cause: Ignored deprecation warnings; postponed library updates beyond recommended window.
- Learnings: Monitor dependency advisories; schedule regular security & compatibility updates; treat 401 after unchanged credentials as a signal to inspect client stack version.

---
## Adding New Issues

Use the following template for real (non-example) issues:

```
### Ticket <ID>
- Status: <Open | In Progress | Resolved>
- Summary: <Concise one-line problem>
- Description: <Detailed context + impact>
- Environment: <OS, toolchain versions, special conditions>
- Date Reported: <YYYY-MM-DD>
- Date Fixed: <YYYY-MM-DD or ->
- Fix Commit: <SHA or link>
- Root Cause: <What actually caused it>
- Learnings: <Actionable lessons>
```

Keep historical accuracy; never overwrite original report dates—append clarifications instead.
