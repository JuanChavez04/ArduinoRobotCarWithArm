import requests
import time

print("========================================")
print("   ESP32 FAIL-FAST IDS / DOS TESTER     ")
print("========================================")

# Get IP Address from Arduino Serial Monitor
ip_address = input("Enter the Robot's IP Address: ")
target_url = f"http://{ip_address}/move?dir=forward"

print(f"\n[+] Target locked: {target_url}")
print("[+] Launching high-speed unauthenticated request flood...\n")

# Rapidly send 12 requests in a row
for i in range(1, 13):
    try:
        # We deliberately send the request WITHOUT a valid session token
        response = requests.get(target_url, timeout=2)
        
        if response.status_code == 403:
            print(f"[{i}] HTTP 403 (Forbidden) -> Request blocked. IDS is logging.")
        elif response.status_code == 429:
            print(f"[{i}] HTTP 429 (Too Many Requests) -> FAIL-FAST TRIGGERED! IP throttled.")
        elif response.status_code == 200:
            print(f"[{i}] HTTP 200 (OK) -> Wait, how did this go through?!")
        else:
            print(f"[{i}] HTTP {response.status_code} -> Unexpected response.")
        
    except requests.exceptions.RequestException as e:
        print(f"[{i}] Network failure. The ESP32 might have crashed: {e}")

print("\n[+] Attack simulation complete.")
