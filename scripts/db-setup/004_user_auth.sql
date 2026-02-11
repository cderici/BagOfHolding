create table if not exists users (
  id bigserial primary key,
  username text not null,
  password_hash text not null,
  is_active boolean not null default true,
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now()
);

create unique index if not exists users_username_lower_uidx on users (lower(username));

create table if not exists auth_sessions (
  id bigserial primary key,
  user_id bigint not null references users(id) on delete cascade,
  refresh_token_hash text not null unique,
  expires_at timestamptz not null,
  revoked_at timestamptz null,
  last_used_at timestamptz null,
  device_name text,
  user_agent text,
  ip inet,
  created_at timestamptz not null default now()
);

-- index for users that have auth sessions
create index if not exists auth_sessions_user_id_idx on auth_sessions(user_id);

-- index for quickly finding expired sessions (expires_at < now())
create index if not exists auth_sessions_expired_at_idx on auth_sessions(expires_at);

-- partial index for active users
create index if not exists auth_sessions_active_used_idx on auth_sessions(user_id) where revoked_at is null;

